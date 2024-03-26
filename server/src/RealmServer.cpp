#include <icon7/RPCEnvironment.hpp>
#include <icon7/Peer.hpp>
#include <icon7/Flags.hpp>

#include "../include/ClientRpcProxy.hpp"
#include "../include/EntityNetworkingSystems.hpp"

#include "../include/RealmServer.hpp"

RealmServer::RealmServer() {}

RealmServer::~RealmServer() {}

void RealmServer::Init(const std::string &realmName)
{
	// TODO: load from database
	// TODO: add to cyclic buffer/queue of realms ; or think about better place
	timer.Start();
	sendEntitiesToClientsTimer.Start();
}

void RealmServer::OneEpoch()
{
	const uint32_t MAX_EVENTS = 128;
	icon7::Command commands[MAX_EVENTS];
	const uint32_t dequeued =
		executionQueue.TryDequeueBulkAny(commands, MAX_EVENTS);

	for (uint32_t i = 0; i < dequeued; ++i) {
		commands[i].Execute();
		commands[i].~Command();
	}

	Realm::OneEpoch();
	// TODO: here do other server updates, AI, other mechanics and logic

	uint64_t dt = 0;
	sendEntitiesToClientsTimer.Update(sendUpdateDeltaTicks, &dt, nullptr);
	if (dt >= sendUpdateDeltaTicks) {
		ClientRpcProxy::Broadcast_UpdateEntities(this);
	}
}

void RealmServer::ConnectPeer(icon7::Peer *peer)
{
	PeerData *data = ((PeerData *)(peer->userPointer));
	data->realm = this;

	uint64_t entityId = NewEntity();
	data->entityId = entityId;
	// TODO: load player from database
	SetComponent<EntityName>(entityId, {data->userName});

	this->EmplaceComponent<EntityPlayerConnectionPeer>(
		entityId, peer->shared_from_this());
	peers.insert(peer);
}

void RealmServer::DisconnectPeer(icon7::Peer *peer)
{
	// TODO: store player in database
	PeerData *data = ((PeerData *)(peer->userPointer));
	uint64_t entityId = data->entityId;
	peers.erase(peer);
	RemoveEntity(entityId);
	data->realm = nullptr;
}

void RealmServer::ExecuteOnRealmThread(
	icon7::Peer *peer, icon7::ByteReader *reader,
	void (*function)(icon7::Peer *, std::vector<uint8_t> &, void *))
{
	icon7::commands::ExecuteOnPeer com;
	std::swap(com.data, reader->_data);
	com.peer = peer->shared_from_this();
	com.userPointer = (void *)(size_t)(reader->get_offset());
	com.function = function;

	executionQueue.EnqueueCommand(std::move(com));
}

void RealmServer::Broadcast(const std::vector<uint8_t> &buffer,
							icon7::Flags flags, uint64_t exceptEntityId)
{
	for (auto it : peers) {
		if (((PeerData *)(it->userPointer))->entityId != exceptEntityId) {
			std::vector<uint8_t> buf = buffer;
			it->Send(std::move(buf), flags);
		}
	}
}

void RealmServer::RegisterObservers()
{
	Realm::RegisterObservers();

	EntityNetworkingSystems::RegisterObservers(this);
}

void RealmServer::RegisterSystems()
{
	Realm::RegisterSystems();

	EntityNetworkingSystems::RegisterSystems(this);

	queryLastAuthoritativeState =
		ecs.query<const EntityLastAuthoritativeMovementState>();

	queryEntityLongState =
		ecs.query<const EntityLastAuthoritativeMovementState, const EntityName,
				   const EntityModelName, const EntityShape,
				   const EntityMovementParameters>();
}
