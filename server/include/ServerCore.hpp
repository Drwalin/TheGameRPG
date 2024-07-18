#pragma once

#include <unordered_map>

#include "../../ICon7/include/icon7/PeerUStcp.hpp"
#include "../../ICon7/include/icon7/Command.hpp"
#include "../../ICon7/include/icon7/RPCEnvironment.hpp"
#include "../../ICon7/include/icon7/Flags.hpp"

#include "RealmServer.hpp"
#include "RealmWorkThreadedManager.hpp"
#include "PeerData.hpp"
#include "CommandParser.hpp"
#include "ConfigStorage.hpp"

class ServerCore
{
public:
	ServerCore();
	~ServerCore();
	void Destroy();

	void CreateRealm(std::string realmName);

	void ConnectPeerToRealm(icon7::Peer *peer);

	void Disconnect(icon7::Peer *peer);

	void StartService();
	void Listen(const std::string &addressInterface, uint16_t port,
				int useIpv4);

	void RunNetworkLoopAsync();
	void RunMainThreadInteractive();

	void BindRpc();

	void Login(icon7::Peer *peer, const std::string &userName);
	static void
	UpdatePlayer(icon7::Peer *peer,
				 const ComponentLastAuthoritativeMovementState &state);
	static void RequestSpawnEntities(icon7::Peer *peer,
									 icon7::ByteReader *reader);
	void InteractInLineOfSight(icon7::Peer *peer, ComponentMovementState state,
							   uint64_t targetId, glm::vec3 dstPos,
							   glm::vec3 normal);
	void Attack(icon7::Peer *peer, ComponentMovementState state,
				uint64_t targetId, glm::vec3 targetPos, int64_t attackType,
				int64_t attackId, const std::string &argStr, int64_t argInt);

private:
	static void _OnPeerConnect(icon7::Peer *peer);
	static void _OnPeerDisconnect(icon7::Peer *peer);

	static icon7::CommandExecutionQueue *
	SelectExecutionQueueByRealm(icon7::MessageConverter *messageConverter,
								icon7::Peer *peer, icon7::ByteReader &reader,
								icon7::Flags flags);

public:
	volatile bool requestStop = false;

	std::unordered_map<std::shared_ptr<icon7::Peer>, PeerData *> peersData;
	std::unordered_map<std::string, std::shared_ptr<icon7::Peer>>
		usernameToPeer;

	RealmWorkThreadedManager realmManager;

	icon7::RPCEnvironment rpc;
	icon7::Host *host;

	std::string spawnRealm;

	CommandParser commandParser;
	ConfigStorage configStorage;
};
