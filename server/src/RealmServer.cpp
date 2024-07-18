#include <icon7/RPCEnvironment.hpp>
#include <icon7/Peer.hpp>
#include <icon7/Flags.hpp>

#include "../../common/include/EntitySystems.hpp"
#include "../../common/include/ClientRpcFunctionNames.hpp"
#include "../../common/include/RegistryComponent.hpp"

#include "../include/CollisionLoader.hpp"
#include "../include/ServerCore.hpp"
#include "../include/ClientRpcProxy.hpp"
#include "../include/EntityNetworkingSystems.hpp"
#include "../include/FileOperations.hpp"
#include "../include/EntityGameComponents.hpp"

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

void RealmServer::ConnectPeer(icon7::Peer *peer)
{
	PeerData *data = ((PeerData *)(peer->userPointer));
	data->realm = weak_from_this();

	uint64_t entityId = NewEntity();

	flecs::entity entity = Entity(entityId);

	auto pw = peer->shared_from_this();
	peers[pw] = entityId;
	SetComponent<ComponentPlayerConnectionPeer>(
		entityId, ComponentPlayerConnectionPeer{peer->shared_from_this()});
	data->entityId = entityId;

	icon7::ByteReader reader(data->storedEntityData, 0);
	if (!(data->storedEntityData.valid() &&
		  data->storedEntityData.size() > 3) &&
		(reader.has_any_more() == false || reader.is_valid() == false)) {
		entity.add<ComponentShape>();
		entity.add<ComponentMovementParameters>();
		ComponentModelName modelName;
		modelName.modelName =
			"characters/low_poly_medieval_people/city_dwellers_1_model.tscn";
		entity.set<ComponentModelName>(modelName);
		entity.add<ComponentEventsQueue>();

		ComponentLastAuthoritativeMovementState s;
		s.oldState.timestamp = timer.currentTick;
		s.oldState.pos = {-87, 0, 54};
		s.oldState.vel = {0, 0, 0};
		s.oldState.onGround = false;
		entity.set<ComponentLastAuthoritativeMovementState>(s);
		entity.set<ComponentMovementState>(s.oldState);
		entity.add<ComponentCharacterSheet>();

		ComponentName name;
		name.name = data->userName;
		SetComponent<ComponentName>(entityId, name);
	} else {
		std::string_view sv;
		reader.op(sv);
		reg::Registry::Singleton().DeserializeAllEntityComponents(entity,
																  reader);
	}

	ClientRpcProxy::SpawnStaticEntities_ForPeer(this, peer);

	icon7::ByteWriter writer(1500);
	ClientRpcProxy::GenericComponentUpdate_Start(this, &writer);
	ClientRpcProxy::GenericComponentUpdate_Update<ComponentCharacterSheet>(
		this, &writer, entity);
	ClientRpcProxy::GenericComponentUpdate_Finish(this, peer, &writer);
}

void RealmServer::DisconnectPeer(icon7::Peer *peer)
{
	PeerData *data = ((PeerData *)(peer->userPointer));
	if (data == nullptr) {
		LOG_ERROR("peer->userPointer is nullptr when shouldn't");
	}
	auto pw = peer->shared_from_this();
	auto it = peers.find(pw);
	if (it != peers.end()) {
		uint64_t entityId = it->second;

		flecs::entity entity = Entity(data->entityId);
		if (entity.is_alive()) {
			if (data->useNextRealmPosition) {
				data->useNextRealmPosition = false;
				auto *_ls =
					entity.get<ComponentLastAuthoritativeMovementState>();
				if (_ls) {
					auto ls = *_ls;
					ls.oldState.pos = data->nextRealmPosition;
					ls.oldState.onGround = false;

					entity.set(ls);

					if (entity.has<ComponentMovementState>()) {
						entity.set(ls.oldState);
					}
				}
			}
		}

		peers.erase(it);

		StorePlayerDataInPeerAndFile(peer);
		RemoveEntity(entityId);
	}
	data->realm.reset();
}

void RealmServer::StorePlayerDataInPeerAndFile(icon7::Peer *peer)
{
	PeerData *data = ((PeerData *)(peer->userPointer));
	if (data == nullptr) {
		LOG_ERROR("peer->userPointer is nullptr when shouldn't");
		return;
	}
	flecs::entity entity = Entity(data->entityId);
	if (entity.is_alive()) {
		if (data->nextRealm == "") {
			auto realm = data->realm.lock();
			data->nextRealm = realm->realmName;
		}

		icon7::ByteWriter writer(std::move(data->storedEntityData));
		if (writer.Buffer().valid() == false) {
			writer.Buffer().Init(4096);
		}
		writer.Buffer().resize(0);
		// TODO: write new realm for player
		writer.op(data->nextRealm);
		reg::Registry::Singleton().SerializeEntity(entity, writer);
		data->storedEntityData = std::move(writer.Buffer());

		std::string filePrefix;
		if (serverCore->configStorage.GetString(
				"config.users_directory_storage.prefix", &filePrefix)) {
		}
		if (FileOperations::WriteFile(filePrefix + data->userName,
									  data->storedEntityData)) {
		} else {
			data->storedEntityData.clear();
		}
	}
}

void RealmServer::ExecuteOnRealmThread(
	icon7::CommandHandle<icon7::Command> &&command)
{
	executionQueue.EnqueueCommand(std::move(command));
}

void RealmServer::Broadcast(icon7::ByteBuffer &buffer, uint64_t exceptEntityId)
{
	for (auto it : peers) {
		if (it.second != exceptEntityId) {
			if (((PeerData *)(it.first->userPointer))->entityId !=
				exceptEntityId) {
				it.first->Send(buffer);
			}
		}
	}
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
			if (peer.peer.get() != nullptr) {
				ClientRpcProxy::SpawnEntities_ForPeerByIdsVector(
					this->shared_from_this(), peer.peer.get(), {entity.id()});
			}
		});
}
