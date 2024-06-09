#include <icon7/RPCEnvironment.hpp>
#include <icon7/Peer.hpp>
#include <icon7/Flags.hpp>

#include "../../common/include/CollisionLoader.hpp"
#include "../../common/include/EntitySystems.hpp"
#include "../../common/include/ClientRpcFunctionNames.hpp"

#include "../include/ClientRpcProxy.hpp"
#include "../include/EntityNetworkingSystems.hpp"

#include "../include/RealmServer.hpp"

RealmServer::RealmServer() { RealmServer::RegisterObservers(); }

RealmServer::~RealmServer() { DisconnectAllAndDestroy(); }

void RealmServer::QueueDestroy() { queueDestroy = true; }

bool RealmServer::IsQueuedToDestroy() { return queueDestroy; }

void RealmServer::DisconnectAllAndDestroy()
{
	// TODO: safe all entities and states
	std::unordered_map<std::shared_ptr<icon7::Peer>, uint64_t> p = this->peers;
	for (auto it : p) {
		DisconnectPeer(it.first.get());
	}
}

uint64_t GE = 0, GE2 = 0;

void RealmServer::Init(const std::string &realmName)
{
	// TODO: load static realm data from database/disk
	Realm::Init(realmName);

	CreateStaticEntity({{}, {0, 0, 0, 1}, {1, 1, 1}},
					   {"map_collision/" + realmName + ".obj"},
					   {"map_collision/" + realmName + ".obj"});

	if (realmName == "MiddleEarth") {
		GE = CreateStaticEntity(
			{{-65, -10, 100}, {0, 0, 0, 1}, {0.1, 0.01, 0.1}},
			{"map_collision/" + realmName + ".obj"},
			{"map_collision/" + realmName + ".obj"});
		GE2 = CreateStaticEntity(
			{{-65, -10, 100}, {1, 0, 0, 0}, {0.1, 0.01, 0.1}},
			{"map_collision/" + realmName + ".obj"},
			{"map_collision/" + realmName + ".obj"});
	}

	sendEntitiesToClientsTimer = 0;
}

bool RealmServer::GetCollisionShape(std::string collisionShapeName,
									TerrainCollisionData *data)
{
	CollisionLoader loader;
	bool res = loader.LoadOBJ(collisionShapeName);
	std::swap(loader.collisionData, *data);
	return res && data->vertices.size() >= 3 && data->indices.size() >= 3;
}

bool RealmServer::OneEpoch()
{
	if (realmName == "MiddleEarth") {

		int64_t y = timer.currentTick;
		y = y % 15000;

		float yv = y / 500.0f;

		flecs::entity e = Entity(GE);
		auto t = (EntityStaticTransform *)e.get<EntityStaticTransform>();
		t->pos.y = yv;
		e.set<EntityStaticTransform>(*t);

		e = Entity(GE2);
		t = (EntityStaticTransform *)e.get<EntityStaticTransform>();
		t->pos.y = yv;
		e.set<EntityStaticTransform>(*t);
	}

	bool busy = executionQueue.Execute(128) != 0;
	busy |= Realm::OneEpoch();
	// TODO: here do other server updates, AI, other mechanics and logic,
	// defer some work to other worker threads (ai, db)  maybe?

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
	data->realm = this->weak_from_this();

	uint64_t entityId = NewEntity();

	flecs::entity entity = Entity(entityId);

	auto pw = peer->shared_from_this();
	peers[pw] = entityId;
	SetComponent<EntityPlayerConnectionPeer>(
		entityId, EntityPlayerConnectionPeer{peer->shared_from_this()});

	entity.add<EntityShape>();
	entity.add<EntityLastAuthoritativeMovementState>();
	entity.add<EntityMovementParameters>();
	entity.set<EntityModelName>(EntityModelName{
		"characters/low_poly_medieval_people/city_dwellers_1_model.tscn"});
	entity.add<EntityEventsQueue>();

	auto s = *entity.get<EntityLastAuthoritativeMovementState>();
	s.oldState.timestamp = timer.currentTick;
	entity.set<EntityLastAuthoritativeMovementState>(s);
	entity.set<EntityMovementState>(s.oldState);

	data->entityId = entityId;
	// TODO: load player entity from database // TODO: move this line into
	// 												   code managed by
	// 												   ServerCore thread
	SetComponent<EntityName>(entityId, EntityName{data->userName});
	LOG_INFO("Client '%s' connected to '%s'", data->userName.c_str(),
			 realmName.c_str());

	ClientRpcProxy::SpawnStaticEntities_ForPeer(this, peer);
}

void RealmServer::DisconnectPeer(icon7::Peer *peer)
{
	PeerData *data = ((PeerData *)(peer->userPointer));
	if (data) {
		data->realm.reset();
	}

	auto pw = peer->shared_from_this();
	auto it = peers.find(pw);
	if (it != peers.end()) {
		uint64_t entityId = it->second;

		peers.erase(pw);

		flecs::entity entity = Entity(entityId);
		if (entity.is_alive()) {
			// TODO: store player entity into database here
		}

		RemoveEntity(entityId);
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

EntityMovementState RealmServer::ExecuteMovementUpdate(uint64_t entityId)
{
	auto entity = Entity(entityId);

	const EntityShape *shape = entity.get<EntityShape>();
	if (shape == nullptr) {
		return {};
	}
	EntityMovementState *currentState =
		(EntityMovementState *)entity.get<EntityMovementState>();
	if (currentState == nullptr) {
		return {};
	}
	const EntityLastAuthoritativeMovementState *lastAuthoritativeState =
		entity.get<EntityLastAuthoritativeMovementState>();
	if (lastAuthoritativeState == nullptr) {
		return {};
	}
	const EntityMovementParameters *movementParams =
		entity.get<EntityMovementParameters>();
	if (movementParams == nullptr) {
		return {};
	}

	EntitySystems::UpdateMovement(this, entity, *shape, *currentState,
								  *lastAuthoritativeState, *movementParams);

	return *currentState;
}

void RealmServer::RegisterObservers()
{
	EntityNetworkingSystems::RegisterObservers(this);

	queryLastAuthoritativeState =
		ecs.query<const EntityLastAuthoritativeMovementState>();

	queryEntityLongState =
		ecs.query<const EntityLastAuthoritativeMovementState, const EntityName,
				  const EntityModelName, const EntityShape,
				  const EntityMovementParameters>();

	queryStaticEntity =
		ecs.query<const EntityStaticTransform, const EntityModelName,
				  const EntityStaticCollisionShapeName>();

	ecs.observer<EntityLastAuthoritativeMovementState>()
		.event(flecs::OnSet)
		.each([this](flecs::entity entity,
					 const EntityLastAuthoritativeMovementState &lastState) {
			BroadcastReliable(ClientRpcFunctionNames::UpdateEntities,
							  entity.id(), lastState);
		});

	ecs.observer<EntityModelName, EntityShape>()
		.event(flecs::OnSet)
		.each([this](flecs::entity entity, const EntityModelName &model,
					 const EntityShape &shape) {
			ClientRpcProxy::Broadcast_SetModel(
				this->shared_from_this(), entity.id(), model.modelName, shape);
		});

	ecs.observer<EntityStaticTransform, EntityModelName,
				 EntityStaticCollisionShapeName>()
		.event(flecs::OnSet)
		.each([this](flecs::entity entity,
					 const EntityStaticTransform &transform,
					 const EntityModelName &model,
					 const EntityStaticCollisionShapeName &shape) {
			// TODO: separate transform broadcast from model and shape broadcast
			ClientRpcProxy::Broadcast_SpawnStaticEntities(
				this, entity, transform, model, shape);
		});
}
