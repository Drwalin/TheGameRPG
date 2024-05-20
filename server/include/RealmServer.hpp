#pragma once

#include "../../ICon7/include/icon7/RPCEnvironment.hpp"
#include "../../ICon7/include/icon7/Peer.hpp"

#include <flecs.h>

#include "PeerData.hpp"

#include "../../common/include/Realm.hpp"
#include "icon7/Flags.hpp"

class ServerCore;

class RealmServer : public Realm
{
public:
	RealmServer();
	virtual ~RealmServer() override;
	
	void DisconnectAllAndDestroy();

	virtual void Init(const std::string &realmName) override;

	// returns false if was not busy
	virtual bool OneEpoch() override;

	void ConnectPeer(icon7::Peer *peer);
	void DisconnectPeer(icon7::Peer *peer);

	void ExecuteOnRealmThread(icon7::CommandHandle<icon7::Command> &&command);

	void RegisterObservers();
	void RegisterSystems();

public:
	void Broadcast(icon7::ByteBuffer &buffer,
				   uint64_t exceptEntityId);

	template <typename... Args>
	void BroadcastReliable(const std::string &functionName, Args&&... args);
	template <typename... Args>
	void BroadcastUnreliable(const std::string &functionName, Args&&... args);
	template <typename... Args>
	void Broadcast(icon7::Flags flags,
			const std::string &functionName, Args&&... args);

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
									Args&&... args)
{
	Broadcast(icon7::FLAG_RELIABLE, functionName, std::forward<Args>(args)...);
}

template <typename... Args>
void RealmServer::BroadcastUnreliable(const std::string &functionName,
									  Args&&... args)
{
	Broadcast(icon7::FLAG_UNRELIABLE, functionName, std::forward<Args>(args)...);
}

template <typename... Args>
void RealmServer::Broadcast(icon7::Flags flags,
		const std::string &functionName, Args&&... args)
{
	icon7::ByteBuffer buffer(1500);
	flags |= icon7::FLAGS_CALL_NO_FEEDBACK;
	rpc->SerializeSend(buffer, flags, functionName, std::forward<Args>(args)...);
	Broadcast(buffer, 0);
}
