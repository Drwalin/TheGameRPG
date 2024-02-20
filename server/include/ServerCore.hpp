#pragma once

#include <cstdio>
#include <cinttypes>

#include <unordered_map>
#include <chrono>

#include <icon7/Command.hpp>
#include <icon7/Peer.hpp>
#include <icon7/Flags.hpp>
#include <icon7/RPCEnvironment.hpp>
#include <icon7/Flags.hpp>

#include "Realm.hpp"
#include "PeerData.hpp"

namespace ClientRemoteFunctions
{
// void UpdateTerrain(TerrainMap)
inline const std::string UpdateTerrain = "UpdateTerrain";

// void SetRealms(std::vector<std::string>)
inline const std::string SetRealms = "SetRealms";

// void SpawnEntities({entityId, lastUpdateTick, vel, pos, forward, height,
// width, maxMovementHorizontalSpeer, movable, modelName, userName}, ...)
inline const std::string SpawnEntities = "SpawnEntities";

// void UpdateEntities({entityId, lastUpdateTick, vel, pos, forward}, ...)
inline const std::string UpdateEntities = "UpdateEntities";

// void SetModel(entityId, modelName, height, width)
inline const std::string SetModel = "SetModel";

// void DeleteEntities({entityId}, ...)
inline const std::string DeleteEntities = "DeleteEntities";

// void SetPlayerEntityId(uint64_t)
inline const std::string SetPlayerEntityId = "SetPlayerEntityId";
} // namespace ClientRemoteFunctions

class ServerCore
{
public:
public:
	ServerCore();
	~ServerCore();

	void CreateRealm(std::string realmName);

	void ConnectPeerToRealm(icon7::Peer *peer, std::string_view realmName);

	void Disconnect(icon7::Peer *peer);

	void StartListening(uint16_t port, int useIpv4);

	void RunNetworkLoopSync();
	void RunNetworkLoopAsync();

	void BindRpc();

	static void SetUsername(icon7::Peer *peer, std::string_view userName);
	static void UpdatePlayer(icon7::Peer *peer, icon7::ByteReader *reader);
	static void RequestSpawnEntities(icon7::Peer *peer,
									 icon7::ByteReader *reader);
	static void GetTerrain(icon7::Peer *peer);
	void RequestRealms(icon7::Peer *peer);

private:
	static void _OnPeerConnect(icon7::Peer *peer);
	static void _OnPeerDisconnect(icon7::Peer *peer);

	static icon7::CommandExecutionQueue *
	SelectExecutionQueue(icon7::MessageConverter *messageConverter,
						 icon7::Peer *peer, icon7::ByteReader &reader,
						 icon7::Flags flags);
	
	static icon7::CommandExecutionQueue *
	SelectExecutionQueueForJoinRealm(icon7::MessageConverter *messageConverter,
						 icon7::Peer *peer, icon7::ByteReader &reader,
						 icon7::Flags flags);

public:
	std::unordered_map<std::shared_ptr<icon7::Peer>, PeerData *> peersData;

	std::unordered_map<std::string, Realm *> realms;
	std::vector<std::string> realmNames;

	icon7::RPCEnvironment rpc;
	icon7::Host *host;
};
