#include <icon7/RPCEnvironment.hpp>
#include <icon7/Peer.hpp>
#include <icon7/Flags.hpp>

#include "../../common/include/CollisionLoader.hpp"
#include "../../common/include/EntitySystems.hpp"

#include "../include/ClientRpcProxy.hpp"
#include "../include/EntityNetworkingSystems.hpp"

#include "../include/RealmServer.hpp"

RealmServer::RealmServer() { RealmServer::RegisterObservers(); }

RealmServer::~RealmServer() { DisconnectAllAndDestroy(); }

void RealmServer::DisconnectAllAndDestroy()
{
	// TODO: safe all entities and states
	std::unordered_map<std::shared_ptr<icon7::Peer>, uint64_t> p = this->peers;
	for (auto it : p) {
		DisconnectPeer(it.first.get());
	}
}

void RealmServer::Init(const std::string &realmName)
{
	// TODO: load static realm data from database/disk
	Realm::Init(realmName);

	CollisionLoader loader;
	loader.LoadOBJ(realmName + ".obj");
	collisionWorld.LoadStaticCollision(&loader.collisionData);

	sendEntitiesToClientsTimer = 0;
}

bool RealmServer::OneEpoch()
{
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
	data->entityId = entityId;
	// TODO: load player entity from database // TODO: move this line into
	// 												   code managed by
	// 												   ServerCore thread
	SetComponent<EntityName>(entityId, {data->userName});

	auto pw = peer->shared_from_this();
	peers[pw] = entityId;
	this->SetComponent<EntityPlayerConnectionPeer>(
		entityId, EntityPlayerConnectionPeer(peer->shared_from_this()));
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
		(EntityMovementState *)
		entity.get<EntityMovementState>();
	if (currentState == nullptr) {
		return {};
	}
	const EntityLastAuthoritativeMovementState
		*lastAuthoritativeState =
		entity.get<EntityLastAuthoritativeMovementState>();
	if (lastAuthoritativeState == nullptr) {
		return {};
	}
	const EntityMovementParameters *movementParams =
		entity.get<EntityMovementParameters>();
	if (movementParams == nullptr) {
		return {};
	}

	EntitySystems::UpdateMovement(
			this, entity, *shape, *currentState,
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
}
