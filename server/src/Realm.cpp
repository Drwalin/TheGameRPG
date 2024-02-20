
#include <chrono>
#include <thread>

#include <bitscpp/ByteReader.hpp>

#include "../include/Realm.hpp"
#include "icon7/Flags.hpp"

Realm::Realm()
{
	lastEntityIdUsed = 0;
	currentTick = 0;
	maxDeltaTicks = 100;
}

Realm::~Realm() {}

void Realm::Init()
{
	terrain.Init(100, 100, 1.0f);
	terrain.Generate(8, 0.8, 500);
}

void Realm::RunAsync()
{
	std::thread([this]() { RunSync(); }).detach();
}

void Realm::RunSync()
{
	const uint32_t MAX_EVENTS = 128;
	icon7::Command commands[MAX_EVENTS];
	const auto startTime = std::chrono::steady_clock::now();
	auto lastUpdateTime = startTime;

	while (true) {
		const uint32_t dequeued =
			executionQueue.TryDequeueBulkAny(commands, MAX_EVENTS);

		for (uint32_t i = 0; i < dequeued; ++i) {
			DEBUG("Executing commands on realm thread");
			commands[i].Execute();
			commands[i].~Command();
		}

		const auto currentTime = std::chrono::steady_clock::now();

		auto diff = std::chrono::duration_cast<std::chrono::milliseconds>(
			currentTime - lastUpdateTime);
		
		if (diff.count() >= maxDeltaTicks) {
			lastUpdateTime= currentTime;
			diff = std::chrono::duration_cast<std::chrono::milliseconds>(
				currentTime - startTime);
			currentTick = diff.count();
			Update();
		} else if (dequeued == 0) {
			std::this_thread::sleep_for(std::chrono::milliseconds(10));
		}
	}
}

void Realm::ConnectToPeer(icon7::Peer *peer)
{
	((PeerData *)(peer->userPointer))->realm = this;
	uint64_t entityId = GenerateNewEntityId();
	Entity &entity = entities[entityId];
	entity.Init(this, entityId);
	entity.ConnectPeer(peer);
	peers.insert(peer);
	rpc->Send(peer, icon7::FLAG_RELIABLE, ClientRemoteFunctions::SetPlayerEntityId, entityId);
	BroadcastSpawnEntity(&entity);
	serverCore->GetTerrain(peer);
}

void Realm::DisconnectPeer(icon7::Peer *peer)
{
	// TODO: safe entity to database
	uint64_t entityId = ((PeerData *)(peer->userPointer))->entityId;
	Entity *entity = GetEntity(entityId);
	if (entity) {
		entity->DisconnectPeer();
	}
	peers.erase(peer);
}

uint64_t Realm::GenerateNewEntityId()
{
	++lastEntityIdUsed;
	while (lastEntityIdUsed == 0 ||
		   entities.find(lastEntityIdUsed) != entities.end()) {
		++lastEntityIdUsed;
	}
	return lastEntityIdUsed;
}

void Realm::SetUpdateDeltaTicks(uint64_t ticks)
{
	maxDeltaTicks = ticks;
	if (maxDeltaTicks < 10) {
		maxDeltaTicks = 10;
	}
}

void Realm::Update()
{
	for (auto &it : entities) {
		it.second.Update(currentTick);
	}
}

void Realm::RequestSpawnEntities(icon7::Peer *peer, icon7::ByteReader *reader)
{
	std::vector<uint8_t> buffer;
	{
		bitscpp::ByteWriter writer(buffer);
		writer.op(ClientRemoteFunctions::SpawnEntities);
		while (reader->get_remaining_bytes() > 8 && reader->is_valid()) {
			uint64_t entityId = 0;
			reader->op(entityId);
			auto it = entities.find(entityId);
			if (it != entities.end()) {
				writer.op(it->second);
			}
		}
	}
	peer->Send(std::move(buffer),
			   icon7::FLAG_RELIABLE | icon7::FLAGS_CALL_NO_FEEDBACK);
}

void Realm::BroadcastSpawnEntity(Entity *entity)
{
	std::vector<uint8_t> buffer;
	{
		bitscpp::ByteWriter writer(buffer);
		writer.op(ClientRemoteFunctions::SpawnEntities);
		writer.op(entity);
	}
	Broadcast(buffer, icon7::FLAG_RELIABLE | icon7::FLAGS_CALL_NO_FEEDBACK, 0);
}

void Realm::SendUpdateEntities(icon7::Peer *peer)
{
	PeerData *data = ((PeerData *)(peer->userPointer));

	const static uint32_t singleEntitySize = 8 + 8 + 12 + 12 + 12;

	std::vector<uint8_t> buffer;

	auto it = entities.begin();
	for (; it != entities.end();) {
		buffer.clear();

		{
			bitscpp::ByteWriter writer(buffer);
			writer.op(ClientRemoteFunctions::UpdateEntities);
			for (; it != entities.end() &&
				   writer.GetSize() + singleEntitySize < 1100;
				 ++it) {
				if (it->second.entityId != data->entityId) {
					it->second.GetUpdateData(writer);
				}
			}
		}

		peer->Send(std::move(buffer),
				   icon7::FLAG_UNRELIABLE | icon7::FLAGS_CALL_NO_FEEDBACK);
	}
}

void Realm::UpdateAllEntities()
{
	const static uint32_t singleEntitySize = 8 + 8 + 12 + 12 + 12;

	std::vector<uint8_t> buffer;

	auto it = entities.begin();
	for (; it != entities.end();) {
		buffer.clear();

		{
			bitscpp::ByteWriter writer(buffer);
			writer.op(ClientRemoteFunctions::UpdateEntities);
			for (; it != entities.end() &&
				   writer.GetSize() + singleEntitySize < 1100;
				 ++it) {
				it->second.GetUpdateData(writer);
			}
		}

		Broadcast(buffer,
				  icon7::FLAG_UNRELIABLE | icon7::FLAGS_CALL_NO_FEEDBACK, 0);
	}
}

void Realm::UpdateModelOf(uint64_t entityId, const std::string &modelName,
						  float height, float width)
{
	std::vector<uint8_t> buffer;
	{
		bitscpp::ByteWriter writer(buffer);
		writer.op(ClientRemoteFunctions::SetModel);
		writer.op(entityId);
		writer.op(modelName);
		writer.op(height);
		writer.op(width);
	}
	Broadcast(buffer, icon7::FLAG_RELIABLE | icon7::FLAGS_CALL_NO_FEEDBACK, 0);
}

void Realm::Broadcast(const std::vector<uint8_t> &buffer, icon7::Flags flags,
					  uint64_t exceptEntityId)
{
	for (auto it : peers) {
		if (((PeerData *)(it->userPointer))->entityId != exceptEntityId) {
			std::vector<uint8_t> buf = buffer;
			it->Send(std::move(buf), flags);
		}
	}
}

void Realm::ExecuteOnRealmThread(icon7::Peer *peer, icon7::ByteReader *reader,
								 void (*function)(icon7::Peer *,
												  std::vector<uint8_t> &,
												  void *))
{
	icon7::commands::ExecuteOnPeer com;
	std::swap(com.data, reader->_data);
	com.peer = peer->shared_from_this();
	com.userPointer = (void *)(size_t)(reader->get_offset());
	com.function = function;

	executionQueue.EnqueueCommand(std::move(com));
}
