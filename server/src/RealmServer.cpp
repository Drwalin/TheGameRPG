#include <icon7/Flags.hpp>
#include <icon7/ByteReader.hpp>

#include "../../common/include/EntitySystems.hpp"
#include "../../common/include/ClientRpcFunctionNames.hpp"
#include "../../common/include/RegistryComponent.hpp"

#define ENABLE_REALM_SERVER_IMPLEMENTATION_TEMPLATE

#include "../include/CollisionLoader.hpp"
#include "../include/ServerCore.hpp"
#include "../include/ClientRpcProxy.hpp"
#include "../include/EntityNetworkingSystems.hpp"
#include "../include/FileOperations.hpp"
#include "../include/EntityGameComponents.hpp"
#include "../include/callbacks/CallbackAiBehaviorTick.hpp"

#include "../include/RealmServer.hpp"

RealmServer::RealmServer() { RealmServer::RegisterObservers(); }

RealmServer::~RealmServer() { DisconnectAllAndDestroy(); }

void RealmServer::QueueDestroy() { queueDestroy = true; }

bool RealmServer::IsQueuedToDestroy() { return queueDestroy; }

void RealmServer::DisconnectAllAndDestroy()
{
	// TODO: safe all entities and states
	std::unordered_map<std::shared_ptr<icon7::Peer>, uint64_t> p = peers;
	for (auto it : p) {
		DisconnectPeer(it.first.get());
	}
}

void RealmServer::Init(const std::string &realmName)
{
	// TODO: load static realm data from database/disk
	Realm::Init(realmName);

	std::string filePrefix, fileSuffix;
	if (serverCore->configStorage.GetString(
			"config.realm_map_file.file_path.prefix", &filePrefix)) {
	}
	if (serverCore->configStorage.GetString(
			"config.realm_map_file.file_path.suffix", &fileSuffix)) {
	}
	const std::string fileName = filePrefix + realmName + fileSuffix;
	icon7::ByteBuffer buffer;
	if (FileOperations::ReadFile(fileName, &buffer)) {
		LOG_INFO("Start loading realm: '%s'  (%s)", realmName.c_str(),
				 fileName.c_str());
		icon7::ByteReader reader(buffer, 0);
		while (reader.get_remaining_bytes() > 10) {
			uint64_t entityId = NewEntity();
			flecs::entity entity = Entity(entityId);
			reg::Registry::Singleton().DeserializeAllEntityComponents(entity,
																	  reader);
			if (reader.is_valid() == false) {
				RemoveEntity(entity);
				break;
			}
		}
		LOG_INFO("Finished loading realm: '%s'", realmName.c_str());
	} else {
		LOG_ERROR("Failed to open map file: '%s'", fileName.c_str());
	}

	sendEntitiesToClientsTimer = 0;
}

bool RealmServer::GetCollisionShape(std::string collisionShapeName,
									TerrainCollisionData *data)
{
	CollisionLoader loader;
	std::string filePrefix;
	if (serverCore->configStorage.GetString(
			"config.collision_shape.file_path.prefix", &filePrefix)) {
		collisionShapeName = filePrefix + collisionShapeName;
	}
	bool res = loader.LoadOBJ(collisionShapeName);
	std::swap(loader.collisionData, *data);
	return res && data->vertices.size() >= 3 && data->indices.size() >= 3;
}

bool RealmServer::OneEpoch()
{
	bool busy = executionQueue.Execute(128) != 0;
	busy |= Realm::OneEpoch();
	// TODO: here do other server updates, AI, other mechanics and logic,
	// defer some work to other worker threads (ai, db)  maybe?

	ecs.each([this](flecs::entity entity, ComponentTrigger &trigger) {
		trigger.Tick(entity.id(), this);
	});

	if (sendEntitiesToClientsTimer + sendUpdateDeltaTicks <=
		timer.currentTick) {
		sendEntitiesToClientsTimer = timer.currentTick;
		ClientRpcProxy::Broadcast_UpdateEntities(shared_from_this());
		return true;
	} else {
		return busy;
	}
}

void RealmServer::ExecuteOnRealmThread(
	icon7::CommandHandle<icon7::Command> &&command)
{
	executionQueue.EnqueueCommand(std::move(command));
}

void RealmServer::ExecuteMovementUpdate(uint64_t entityId,
										ComponentMovementState *state)
{
	auto entity = Entity(entityId);

	const ComponentShape *shape = entity.get<ComponentShape>();
	if (shape == nullptr) {
		*state = {};
		return;
	}
	ComponentMovementState *currentState =
		(ComponentMovementState *)entity.get<ComponentMovementState>();
	if (currentState == nullptr) {
		*state = {};
		return;
	}
	const ComponentLastAuthoritativeMovementState *lastAuthoritativeState =
		entity.get<ComponentLastAuthoritativeMovementState>();
	if (lastAuthoritativeState == nullptr) {
		*state = {};
		return;
	}
	const ComponentMovementParameters *movementParams =
		entity.get<ComponentMovementParameters>();
	if (movementParams == nullptr) {
		*state = {};
		return;
	}

	EntitySystems::UpdateMovement(this, entity, *shape, *currentState,
								  *lastAuthoritativeState, *movementParams);

	*state = *currentState;
}

void RealmServer::RegisterObservers()
{
	EntityNetworkingSystems::RegisterObservers(this);

	queryLastAuthoritativeState =
		ecs.query<const ComponentLastAuthoritativeMovementState>();

	queryEntityLongState =
		ecs.query<const ComponentLastAuthoritativeMovementState,
				  const ComponentName, const ComponentModelName,
				  const ComponentShape, const ComponentMovementParameters>();

	queryStaticEntity =
		ecs.query<const ComponentStaticTransform, const ComponentModelName,
				  const ComponentStaticCollisionShapeName>();

	ecs.observer<ComponentLastAuthoritativeMovementState>()
		.event(flecs::OnSet)
		.each([this](flecs::entity entity,
					 const ComponentLastAuthoritativeMovementState &lastState) {
			BroadcastReliable(ClientRpcFunctionNames::UpdateEntities,
							  entity.id(), lastState);
		});

	ecs.observer<ComponentModelName, ComponentShape>()
		.event(flecs::OnSet)
		.each([this](flecs::entity entity, const ComponentModelName &model,
					 const ComponentShape &shape) {
			ClientRpcProxy::Broadcast_SetModel(shared_from_this(), entity.id(),
											   model.modelName, shape);
		});

	ecs.observer<ComponentStaticTransform, ComponentModelName,
				 ComponentStaticCollisionShapeName>()
		.event(flecs::OnSet)
		.each([this](flecs::entity entity,
					 const ComponentStaticTransform &transform,
					 const ComponentModelName &model,
					 const ComponentStaticCollisionShapeName &shape) {
			// TODO: separate transform broadcast from model and shape broadcast
			ClientRpcProxy::Broadcast_SpawnStaticEntities(
				this, entity, transform, model, shape);
		});

	ecs.observer<const ComponentStaticTransform, ComponentTrigger>()
		.event(flecs::OnAdd)
		.each([this](flecs::entity entity,
					 const ComponentStaticTransform &transform,
					 ComponentTrigger &trigger) {
			collisionWorld.OnAddTrigger(entity, transform);
		});

	ecs.observer<ComponentTrigger>()
		.event(flecs::OnSet)
		.each([this](flecs::entity entity, ComponentTrigger &trigger) {
			trigger.tickUntilIgnore = timer.currentTick + 1000;
		});

	ecs.observer<ComponentEventsQueue, const ComponentAITick>()
		.event(flecs::OnAdd)
		.each([this](flecs::entity entity, ComponentEventsQueue &eventsQueue,
					 const ComponentAITick &) {
			static EntityEventTemplate defaultAiMovementEvent{
				[](Realm *realm, int64_t scheduledTick, int64_t currentTick,
				   uint64_t entityId) {
					flecs::entity entity = realm->Entity(entityId);
					if (entity.has<ComponentAITick>()) {
						auto tick = entity.get<ComponentAITick>();
						if (tick) {
							if (tick->aiTick) {
								tick->aiTick->Call((RealmServer *)realm,
												   entityId);
							}
						}
					}

					int64_t dt = realm->maxMovementDeltaTicks;

					EntityEvent event;
					event.dueTick = realm->timer.currentTick + dt;
					event.event = &defaultAiMovementEvent;

					ComponentEventsQueue *eventsQueue =
						realm->AccessComponent<ComponentEventsQueue>(entityId);
					if (eventsQueue == nullptr) {
						LOG_FATAL("Events queue removed but event AIUpdate was "
								  "executed.");
						return;
					}

					eventsQueue->ScheduleEvent(realm, entityId, event);
				}};
			EntityEvent event;
			event.dueTick = timer.currentTick + 100;
			event.event = &defaultAiMovementEvent;
			eventsQueue.ScheduleEvent(this, entity.id(), event);
		});

	ecs.observer<ComponentLastAuthoritativeMovementState>()
		.event(flecs::OnAdd)
		.each([](flecs::entity entity,
				 const ComponentLastAuthoritativeMovementState &) {
			if (entity.has<ComponentEventsQueue>() == false) {
				entity.add<ComponentEventsQueue>();
			}
		});
	ecs.observer<ComponentMovementParameters>()
		.event(flecs::OnAdd)
		.each([](flecs::entity entity, const ComponentMovementParameters &) {
			if (entity.has<ComponentEventsQueue>() == false) {
				entity.add<ComponentEventsQueue>();
			}
		});
	ecs.observer<ComponentMovementState>()
		.event(flecs::OnAdd)
		.each([](flecs::entity entity, const ComponentMovementState &) {
			if (entity.has<ComponentEventsQueue>() == false) {
				entity.add<ComponentEventsQueue>();
			}
		});

	ecs.observer<ComponentAITick>()
		.event(flecs::OnAdd)
		.each([](flecs::entity entity, const ComponentAITick &) {
			if (entity.has<ComponentEventsQueue>() == false) {
				entity.add<ComponentEventsQueue>();
			}
		});

	ecs.observer<ComponentLastAuthoritativeMovementState,
				 ComponentPlayerConnectionPeer>()
		.event(flecs::OnSet)
		.each([this](flecs::entity entity,
					 const ComponentLastAuthoritativeMovementState &state,
					 ComponentPlayerConnectionPeer &peer) {
			if (currentlyUpdatingPlayerPeerEntityMovement == false) {
				if (peer.peer.get() != nullptr) {
					ClientRpcProxy::SpawnEntities_ForPeerByIdsVector(
						this->shared_from_this(), peer.peer.get(),
						{entity.id()});
				}
			}
		});
}
