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

RealmServer::~RealmServer() {}

void RealmServer::Init(const std::string &realmName)
{
	// TODO: load static realm data from database/disk
	Realm::Init(realmName);
	sendEntitiesToClientsTimer.Start();
}

bool RealmServer::OneEpoch()
{
	bool busy = false;
	const uint32_t MAX_EVENTS = 128;
	icon7::Command commands[MAX_EVENTS];
	const uint32_t dequeued =
		executionQueue.TryDequeueBulkAny(commands, MAX_EVENTS);

	busy |= dequeued != 0;
	for (uint32_t i = 0; i < dequeued; ++i) {
		commands[i].Execute();
		commands[i].~Command();
	}

	busy |= Realm::OneEpoch();
	// TODO: here do other server updates, AI, other mechanics and logic

	uint64_t dt = 0;
	sendEntitiesToClientsTimer.Update(sendUpdateDeltaTicks, &dt, nullptr);
	if (dt >= sendUpdateDeltaTicks) {
		ClientRpcProxy::Broadcast_UpdateEntities(this);
		return true;
	} else {
		return busy;
	}
}

void RealmServer::ConnectPeer(icon7::Peer *peer)
{
	PeerData *data = ((PeerData *)(peer->userPointer));
	data->realm = this;

	uint64_t entityId = NewEntity();
	data->entityId = entityId;
	// TODO: load player entity from database
	SetComponent<EntityName>(entityId, {data->userName});

	peers[peer] = entityId;
	this->SetComponent<EntityPlayerConnectionPeer>(
		entityId, EntityPlayerConnectionPeer(peer->shared_from_this()));
}

void RealmServer::DisconnectPeer(icon7::Peer *peer)
{
	// TODO: store player entity into database
	PeerData *data = ((PeerData *)(peer->userPointer));
	if (data) {
		data->realm = nullptr;
	}

	auto it = peers.find(peer);
	if (it != peers.end()) {
		uint64_t entityId = it->second;
		peers.erase(peer);
		RemoveEntity(entityId);
	}
}

void RealmServer::ExecuteOnRealmThread(
	icon7::Peer *peer, std::vector<uint8_t> &customData,
	void (*function)(icon7::Peer *, std::vector<uint8_t> &, void *))
{
	icon7::commands::ExecuteOnPeer com;
	std::swap(com.data, customData);
	com.peer = peer->shared_from_this();
	com.userPointer = this;
	com.function = function;

	executionQueue.EnqueueCommand(std::move(com));
}

void RealmServer::Broadcast(const std::vector<uint8_t> &buffer,
							icon7::Flags flags, uint64_t exceptEntityId)
{
	for (auto it : peers) {
		if (it.second != exceptEntityId) {
			if (((PeerData *)(it.first->userPointer))->entityId !=
				exceptEntityId) {
				std::vector<uint8_t> buf = buffer;
				it.first->Send(std::move(buf), flags);
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
