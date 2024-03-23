
#include <icon7/RPCEnvironment.hpp>
#include <icon7/Peer.hpp>

#include "../include/RealmServer.hpp"
#include "icon7/Flags.hpp"

RealmServer::RealmServer() {}

RealmServer::~RealmServer() {}

void RealmServer::InitByRealmName(const std::string &realmName)
{
	// TODO: load from database
	// TODO: add to cyclic buffer/queue of realms ; or think about better place
	timer.Start();
	sendEntitiesToClientsTimer.Start();
}

void RealmServer::Update()
{
	const uint32_t MAX_EVENTS = 128;
	icon7::Command commands[MAX_EVENTS];
	const uint32_t dequeued =
		executionQueue.TryDequeueBulkAny(commands, MAX_EVENTS);

	for (uint32_t i = 0; i < dequeued; ++i) {
		commands[i].Execute();
		commands[i].~Command();
	}

	uint64_t deltaTicks = 0;
	timer.Update(maxDeltaTicks, &deltaTicks, nullptr);

	if (deltaTicks >= maxDeltaTicks) {
		UpdateAllEntities();
		uint64_t dt = 0;
		sendEntitiesToClientsTimer.Update(sendUpdateDeltaTicks, &dt, nullptr);
		if (dt >= sendUpdateDeltaTicks) {
			BroadcastEntitiesMovementState();
		}
	}
}

void RealmServer::UpdateAllEntities()
{
	entityStorage.ForEach(
		[this](EntityServer *e) { e->Update(timer.currentTick); });
}

void RealmServer::AddPeer(icon7::Peer *peer)
{
	((PeerData *)(peer->userPointer))->realm = this;

	uint64_t entityId = AddEntity();
	EntityServer *entity = entityStorage.Get(entityId);
	entity->ConnectPeer(peer);
	peers.insert(peer);
	// TODO: send this player entity controlled id, send realm data
	rpc->Send(peer, icon7::FLAG_RELIABLE,
			  ClientRemoteFunctions::SetPlayerEntityId, entityId);
}

void RealmServer::DisconnectPeer(icon7::Peer *peer)
{
	// TODO: safe entity to database
	uint64_t entityId = ((PeerData *)(peer->userPointer))->entityId;
	EntityServer *entity = (EntityServer *)GetEntity(entityId);
	if (entity) {
		entity->DisconnectPeer();
	}
	peers.erase(peer);
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

uint64_t RealmServer::_InternalAddEntity()
{
	uint64_t entityId = entityStorage.Add(this);
	BroadcastEntityLongState(entityId);
	return entityId;
}

EntityBase *RealmServer::GetEntity(uint64_t entityId)
{
	return entityStorage.Get(entityId);
}

uint64_t RealmServer::_InternalDestroyEntity(uint64_t entityId)
{
	EntityServer *entity = entityStorage.Get(entityId);
	entity->DisconnectPeer();
	// TODO: propagate entity destruction
	entityStorage.Destroy(entityId);
	BroadcastEntityErase(entityId);
}

void RealmServer::BroadcastEntitiesMovementState()
{
	const static uint32_t singleEntitySize = 8 + 8 + 12 + 12 + 12 + 1;

	std::vector<uint8_t> buffer;

	auto it = entityStorage.entities.begin();
	for (; it != entityStorage.entities.end();) {
		buffer.clear();

		{
			bitscpp::ByteWriter writer(buffer);
			writer.op(ClientRemoteFunctions::UpdateEntities);
			for (; it != entityStorage.entities.end() &&
				   writer.GetSize() + singleEntitySize < 1100;
				 ++it) {
				writer.op(it->first);
				writer.op(it->second.lastAuthoritativeState);
			}
		}

		Broadcast(buffer,
				  icon7::FLAG_UNRELIABLE | icon7::FLAGS_CALL_NO_FEEDBACK, 0);
	}
}

void RealmServer::BroadcastEntityChangeModel(uint64_t entityId)
{
	EntityBase *entity = GetEntity(entityId);
	if (entity == nullptr) {
		return;
	}
	BroadcastReliable(ClientRemoteFunctions::ChangeModel, entityId,
					  entity->longState.modelName,
					  entity->longState.shape.width,
					  entity->longState.shape.height);
}

void RealmServer::BroadcastEntityLongState(uint64_t entityId)
{
	EntityBase *entity = GetEntity(entityId);
	if (entity == nullptr) {
		return;
	}
	BroadcastReliable(ClientRemoteFunctions::EntityLongState, entity->longState);
}

void RealmServer::BroadcastEntityErase(uint64_t entityId)
{
	EntityBase *entity = GetEntity(entityId);
	if (entity == nullptr) {
		return;
	}
	BroadcastReliable(ClientRemoteFunctions::EntityErase, entityId);
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

void RealmServer::RequestLongEntitiesData(icon7::Peer *peer, icon7::ByteReader *reader)
{
	std::vector<uint8_t> buffer;
	{
		bitscpp::ByteWriter writer(buffer);
		writer.op(ClientRemoteFunctions::EntityLongState);
		while (reader->get_remaining_bytes() > 8 && reader->is_valid()) {
			uint64_t entityId = 0;
			reader->op(entityId);
			auto entity = entityStorage.Get(entityId);
			if (entity) {
				writer.op(entity->longState);
			}
		}
	}
	peer->Send(std::move(buffer),
			   icon7::FLAG_RELIABLE | icon7::FLAGS_CALL_NO_FEEDBACK);
}
