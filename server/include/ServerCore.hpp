#pragma once

#include <unordered_map>

#include "../../ICon7/include/icon7/Flags.hpp"
#include "../../ICon7/include/icon7/Forward.hpp"
#include "../../ICon7/include/icon7/CoroutineHelper.hpp"
#include "../../ICon7/include/icon7/CommandExecutionQueue.hpp"

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

	icon7::CoroutineSchedulable ConnectPeerToRealm(icon7::PeerHandle peer);

	void Disconnect(icon7::PeerHandle peer);

	void StartService();
	void Listen(const std::string &addressInterface, uint16_t port,
				int useIpv4);

	void RunNetworkLoopAsync();
	void RunMainThreadInteractive();

	void BindRpc();

	void Login(icon7::PeerHandle peer, const std::string &userName);
	static void UpdatePlayer(icon7::CommandExecutionQueue *queue, icon7::PeerHandle peer,
							 const ComponentMovementState &state);
	static void RequestSpawnEntities(icon7::PeerHandle peer,
									 icon7::ByteReader *reader);
	void InteractInLineOfSight(icon7::PeerHandle peer, ComponentMovementState state,
							   uint64_t targetId, glm::vec3 dstPos,
							   glm::vec3 normal);
	void Attack(icon7::PeerHandle peer, ComponentMovementState state,
				uint64_t targetId, glm::vec3 targetPos, int64_t attackType,
				int64_t attackId, int64_t argInt);

	void RemoveDeadPlayerNicknameAfterDestroyingEntity_Async(icon7::PeerHandle peer);

	static icon7::CommandExecutionQueue::CoroutineAwaitable
	ScheduleInRealm(std::weak_ptr<RealmServer> &realm);
	icon7::CommandExecutionQueue::CoroutineAwaitable
	ScheduleInRealmOrCore(std::weak_ptr<RealmServer> &realm);

private:
	void _OnPeerConnect(icon7::PeerHandle peer);
	void _OnPeerDisconnect(icon7::PeerHandle peer);

	icon7::CommandExecutionQueue *
	SelectExecutionQueueByRealm(icon7::MessageConverter *messageConverter,
								icon7::PeerHandle peer, icon7::ByteReader &reader,
								icon7::Flags flags);

public:
	volatile bool requestStop = false;

	std::unordered_map<icon7::PeerHandle, std::shared_ptr<PeerData>> peerHandleToData;
	std::unordered_map<std::string, std::shared_ptr<icon7::PeerData>> usernameToPeer;

	RealmWorkThreadedManager realmManager;

	icon7::RPCEnvironment *rpc;
	std::shared_ptr<icon7::Host> host;
	std::shared_ptr<icon7::Loop> loop;

	std::string spawnRealm;

	CommandParser commandParser;
	ConfigStorage configStorage;
};
