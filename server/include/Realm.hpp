#pragma once

#include <cstdio>
#include <cinttypes>

#include <unordered_map>
#include <chrono>

#include <glm/glm.hpp>

#include <icon7/Peer.hpp>
#include <icon7/CommandExecutionQueue.hpp>
#include "icon7/RPCEnvironment.hpp"

#include "Entity.hpp"
#include "TerrainMap.hpp"
#include "ServerCore.hpp"
#include "PeerData.hpp"
#include "Timer.hpp"

class ServerCore;

#define ON_REALM_THREAD_FUNCTION_CREATOR_EXECUTION_HELPER_WITH_DATA(REALM, PEER, READER, SRC) \
	if (REALM) { \
		REALM->ExecuteOnRealmThread(PEER, READER, \
		+[](icon7::Peer *peer, std::vector<uint8_t> &data, void *ptr) { \
			PeerData *userData = ((PeerData *)(peer->userPointer)); \
			if (userData->realm) { \
				uint32_t offset = (uint32_t)(size_t)ptr; \
				icon7::ByteReader reader(data, offset); \
				SRC \
			} \
		}); \
	}

#define ON_REALM_THREAD_FUNCTION_CREATOR_EXECUTION_HELPER_WITHOUT_DATA(REALM, PEER, SRC) \
	if (REALM) { \
		REALM->ExecuteOnRealmThread(PEER, nullptr, \
		+[](icon7::Peer *peer, std::vector<uint8_t> &data, void *ptr) { \
			PeerData *userData = ((PeerData *)(peer->userPointer)); \
			if (userData->realm) { \
				SRC \
			} \
		}); \
	}

class Realm
{
public:
	Realm();
	~Realm();

	void Init();
	void RunAsync();
	void RunSync();

	void ConnectToPeer(icon7::Peer *peer);
	void DisconnectPeer(icon7::Peer *peer);

	uint64_t GenerateNewEntityId();

	inline uint64_t GetCurrentTick() const { return timer.currentTick; }

	void SetUpdateDeltaTicks(uint64_t ticks);

	inline Entity *GetEntity(uint64_t id)
	{
		auto it = entities.find(id);
		if (it == entities.end()) {
			return nullptr;
		}
		return &(it->second);
	}

	ServerCore *serverCore;

	std::string realmName;
	icon7::RPCEnvironment *rpc;
	icon7::CommandExecutionQueue executionQueue;

	void RequestSpawnEntities(icon7::Peer *peer, icon7::ByteReader *reader);
	void BroadcastSpawnEntity(Entity *entity);
	void SendUpdateEntities(icon7::Peer *peer);
	void SendUpdateAllEntities();

	void UpdateModelOf(uint64_t entityId, const std::string &modelName,
					   float height, float width);
	
	void Broadcast(const std::vector<uint8_t> &buffer, icon7::Flags flags, uint64_t exceptEntityId);
	
	void ExecuteOnRealmThread(icon7::Peer *peer, icon7::ByteReader *reader, void(*function)(icon7::Peer *, std::vector<uint8_t> &, void *));

	friend class ServerCore;
	
	Timer timer;

public:
	TerrainMap terrain;
	float gravity = 9.81;

private:
	void Update();

private:

	uint64_t maxDeltaTicks=100;
	uint64_t sendUpdateDeltaTicks=250;

	std::unordered_map<uint64_t, Entity> entities;
	std::unordered_set<icon7::Peer *> peers;

	uint64_t lastEntityIdUsed;
};
