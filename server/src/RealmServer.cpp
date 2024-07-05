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
	std::string fileName = filePrefix + realmName + fileSuffix;
	icon7::ByteBuffer buffer;
	if (FileOperations::ReadFile(fileName, &buffer)) {
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
		entity.add<ComponentLastAuthoritativeMovementState>();
		entity.add<ComponentMovementParameters>();
		ComponentModelName modelName;
		modelName.modelName =
			"characters/low_poly_medieval_people/city_dwellers_1_model.tscn";
		entity.set<ComponentModelName>(modelName);
		entity.add<ComponentEventsQueue>();

		auto s = *entity.get<ComponentLastAuthoritativeMovementState>();
		s.oldState.timestamp = timer.currentTick;
		s.oldState.pos = {-87, 0, 54};
		entity.set<ComponentLastAuthoritativeMovementState>(s);
		entity.set<ComponentMovementState>(s.oldState);

		ComponentName name;
		name.name = data->userName;
		SetComponent<ComponentName>(entityId, name);
	} else {
		std::string_view sv;
		reader.op(sv);
		reg::Registry::Singleton().DeserializeAllEntityComponents(entity,
																  reader);
	}

	// 	LOG_INFO("Client '%s' connected to '%s'", data->userName.c_str(),
	// 			 realmName.c_str());

	ClientRpcProxy::SpawnStaticEntities_ForPeer(this, peer);

	if (data->useNextRealmPosition) {
		auto *_ls = entity.get<ComponentLastAuthoritativeMovementState>();
		if (_ls) {
			auto ls = *_ls;
			ls.oldState.pos = data->nextRealmlPosition;
			entity.set<ComponentLastAuthoritativeMovementState>(ls);
		}

		auto *_ms = entity.get<ComponentMovementState>();
		if (_ms) {
			auto ms = *_ms;
			ms.pos = data->nextRealmlPosition;
			entity.set<ComponentMovementState>(ms);
		}

		data->useNextRealmPosition = false;
	}
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
}
