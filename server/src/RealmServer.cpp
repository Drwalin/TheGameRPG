#include <icon7/RPCEnvironment.hpp>
#include <icon7/Peer.hpp>
#include <icon7/Flags.hpp>

#include "../include/ClientRpcProxy.hpp"
#include "../include/EntityNetworkingSystems.hpp"

#include "../include/RealmServer.hpp"

RealmServer::RealmServer()
{
	RealmServer::RegisterSystems();
	RealmServer::RegisterObservers();
}

RealmServer::~RealmServer() {
	DisconnectAllAndDestroy();
}

void RealmServer::DisconnectAllAndDestroy()
{
	// TODO: here
}

void RealmServer::Init(const std::string &realmName)
{
	// TODO: load static realm data from database/disk
	Realm::Init(realmName);
	sendEntitiesToClientsTimer.Start();
}

bool RealmServer::OneEpoch()
{
	bool busy = executionQueue.Execute(128) != 0;
	busy |= Realm::OneEpoch();
	// TODO: here do other server updates, AI, other mechanics and logic,
	// defer some work to other worker threads (ai, db)  maybe?

	int64_t dt = 0;
	sendEntitiesToClientsTimer.Update(sendUpdateDeltaTicks, &dt, nullptr);
	if (dt >= sendUpdateDeltaTicks) {
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
	LOG_DEBUG("Spawn   entity: [%lu]", entityId);
	data->entityId = entityId;
	// TODO: load player entity from database
	SetComponent<EntityName>(entityId, {data->userName});

	peers[peer] = entityId;
	this->SetComponent<EntityPlayerConnectionPeer>(
		entityId, EntityPlayerConnectionPeer(peer->shared_from_this()));
}

void RealmServer::DisconnectPeer(icon7::Peer *peer)
{
	PeerData *data = ((PeerData *)(peer->userPointer));
	if (data) {
		data->realm.reset();
	}

	auto it = peers.find(peer);
	if (it != peers.end()) {
		uint64_t entityId = it->second;
		
		peers.erase(peer);
		
		flecs::entity entity = Entity(entityId);
		if (entity.is_alive()) {
			// store player entity into database
			StoreEntityIntoDatabase(entity);
		}
		
		RemoveEntity(entityId);
	}
}

void RealmServer::StoreEntityIntoDatabase(flecs::entity entity)
{
	// TODO: implement
}

void RealmServer::ExecuteOnRealmThread(
	icon7::CommandHandle<icon7::Command> &&command)
{
	executionQueue.EnqueueCommand(std::move(command));
}

void RealmServer::Broadcast(icon7::ByteBuffer &buffer,
		uint64_t exceptEntityId)
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

void RealmServer::RegisterObservers()
{
	EntityNetworkingSystems::RegisterObservers(this);
}

void RealmServer::RegisterSystems()
{
	EntityNetworkingSystems::RegisterSystems(this);

	queryLastAuthoritativeState =
		ecs.query<const EntityLastAuthoritativeMovementState>();

	queryEntityLongState =
		ecs.query<const EntityLastAuthoritativeMovementState, const EntityName,
				  const EntityModelName, const EntityShape,
				  const EntityMovementParameters>();
}
