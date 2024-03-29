#pragma once

#include <unordered_map>

#include <icon7/Command.hpp>
#include <icon7/Peer.hpp>
#include <icon7/Flags.hpp>
#include <icon7/RPCEnvironment.hpp>
#include <icon7/Flags.hpp>

#include "RealmServer.hpp"
#include "RealmWorkThreadedManager.hpp"
#include "PeerData.hpp"

class ServerCore
{
public:
public:
	ServerCore();
	~ServerCore();

	void CreateRealm(std::string realmName);

	void ConnectPeerToRealm(icon7::Peer *peer, std::string realmName);

	void Disconnect(icon7::Peer *peer);

	void StartListening(uint16_t port, int useIpv4);

	void RunNetworkLoopSync();
	void RunNetworkLoopAsync();

	void BindRpc();

	static void SetUsername(icon7::Peer *peer, std::string_view userName);
	static void UpdatePlayer(icon7::Peer *peer, const EntityLastAuthoritativeMovementState &state);
	static void RequestSpawnEntities(icon7::Peer *peer,
									 icon7::ByteReader *reader);
	void GetCurrentTick(icon7::Peer *peer, icon7::Flags flags);

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

	RealmWorkThreadedManager realmManager;

	icon7::RPCEnvironment rpc;
	icon7::Host *host;
	
	std::string spawnRealm;
};
