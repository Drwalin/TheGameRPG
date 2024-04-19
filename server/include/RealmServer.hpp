#pragma once

#include <vector>

#include "../../ICon7-godot-client/ICon7/include/icon7/RPCEnvironment.hpp"
#include "../../ICon7-godot-client/ICon7/include/icon7/Peer.hpp"

#include <flecs.h>

#include "PeerData.hpp"

#include "../../common/include/Realm.hpp"

class ServerCore;

class RealmServer : public Realm
{
public:
	RealmServer();
	virtual ~RealmServer() override;

	virtual void Init(const std::string &realmName) override;

	// returns false if was not busy
	virtual bool OneEpoch() override;

	void ConnectPeer(icon7::Peer *peer);
	void DisconnectPeer(icon7::Peer *peer);

	void ExecuteOnRealmThread(icon7::Peer *peer,
							  std::vector<uint8_t> &customData,
							  void (*function)(icon7::Peer *,
											   std::vector<uint8_t> &, void *));

	void RegisterObservers();
	void RegisterSystems();

public:
	void Broadcast(const std::vector<uint8_t> &buffer, icon7::Flags flags,
				   uint64_t exceptEntityId);

	template <typename... Args>
	void BroadcastReliable(const std::string &functionName, Args... args);
	template <typename... Args>
	void BroadcastUnreliable(const std::string &functionName, Args... args);

public:
	ServerCore *serverCore;
	icon7::RPCEnvironment *rpc;
	icon7::CommandExecutionQueue executionQueue;

	std::unordered_map<icon7::Peer *, uint64_t> peers;

	Timer sendEntitiesToClientsTimer;
	int64_t sendUpdateDeltaTicks = 250;
	flecs::query<const EntityLastAuthoritativeMovementState>
		queryLastAuthoritativeState;

	flecs::query<const EntityLastAuthoritativeMovementState, const EntityName,
				 const EntityModelName, const EntityShape,
				 const EntityMovementParameters>
		queryEntityLongState;
};

template <typename... Args>
void RealmServer::BroadcastReliable(const std::string &functionName,
									Args... args)
{
	std::vector<uint8_t> buffer;
	icon7::Flags flags = icon7::FLAG_RELIABLE | icon7::FLAGS_CALL_NO_FEEDBACK;
	{
		bitscpp::ByteWriter writer(buffer);
		icon7::RPCEnvironment::SerializeSend(writer, flags, functionName,
											 args...);
	}
	Broadcast(buffer, flags, 0);
}

template <typename... Args>
void RealmServer::BroadcastUnreliable(const std::string &functionName,
									  Args... args)
{
	std::vector<uint8_t> buffer;
	icon7::Flags flags = icon7::FLAG_UNRELIABLE | icon7::FLAGS_CALL_NO_FEEDBACK;
	{
		bitscpp::ByteWriter writer(buffer);
		icon7::RPCEnvironment::SerializeSend(writer, flags, functionName,
											 args...);
	}
	Broadcast(buffer, flags, 0);
}
