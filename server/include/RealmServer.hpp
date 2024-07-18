#pragma once

#include "../../ICon7/include/icon7/Flags.hpp"
#include "../../ICon7/include/icon7/RPCEnvironment.hpp"
#include "../../ICon7/include/icon7/CommandExecutionQueue.hpp"
#include "../../ICon7/include/icon7/Peer.hpp"

#include "PeerData.hpp"

#include "../../common/include/Realm.hpp"

class ServerCore;

class RealmServer : public Realm,
					public std::enable_shared_from_this<RealmServer>
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

	virtual void ExecuteMovementUpdate(uint64_t entityId,
									   ComponentMovementState *state) override;

	void RegisterObservers();

	virtual bool GetCollisionShape(std::string collisionShapeName,
								   TerrainCollisionData *data) override;

	void QueueDestroy();
	bool IsQueuedToDestroy();

public: // Entity Actions
	void InteractInLineOfSight(uint64_t instigatorId,
							   ComponentMovementState state, uint64_t targetId,
							   glm::vec3 dstPos, glm::vec3 normal);
	void Attack(uint64_t instigatorId, ComponentMovementState state,
				uint64_t targetId, glm::vec3 targetPos, int64_t attackType,
				int64_t attackId, const std::string &argStr, int64_t argInt);

	void InteractInLineOfSight(icon7::Peer *peer, ComponentMovementState state,
							   uint64_t targetId, glm::vec3 dstPos,
							   glm::vec3 normal);
	void Attack(icon7::Peer *peer, ComponentMovementState state,
				uint64_t targetId, glm::vec3 targetPos, int64_t attackType,
				int64_t attackId, const std::string &argStr, int64_t argInt);

private:
	void StorePlayerDataInPeerAndFile(icon7::Peer *peer);

public:
	void Broadcast(icon7::ByteBuffer &buffer, uint64_t exceptEntityId);

	template <typename... Args>
	void BroadcastReliable(const std::string &functionName, Args &&...args);
	template <typename... Args>
	void BroadcastUnreliable(const std::string &functionName, Args &&...args);
	template <typename... Args>
	void Broadcast(icon7::Flags flags, const std::string &functionName,
				   Args &&...args);

	template <typename... Args>
	void BroadcastReliableExcept(uint64_t exceptEntityId,
								 const std::string &functionName,
								 Args &&...args);
	template <typename... Args>
	void BroadcastUnreliableExcept(uint64_t exceptEntityId,
								   const std::string &functionName,
								   Args &&...args);
	template <typename... Args>
	void BroadcastExcept(uint64_t exceptEntityId, icon7::Flags flags,
						 const std::string &functionName, Args &&...args);

public:
	ServerCore *serverCore;
	icon7::RPCEnvironment *rpc;
	icon7::CommandExecutionQueue executionQueue;

	std::unordered_map<std::shared_ptr<icon7::Peer>, uint64_t> peers;

	uint64_t sendEntitiesToClientsTimer = 0;
	int64_t sendUpdateDeltaTicks = 250;
	flecs::query<const ComponentLastAuthoritativeMovementState>
		queryLastAuthoritativeState;

	flecs::query<const ComponentLastAuthoritativeMovementState,
				 const ComponentName, const ComponentModelName,
				 const ComponentShape, const ComponentMovementParameters>
		queryEntityLongState;

	flecs::query<const ComponentStaticTransform, const ComponentModelName,
				 const ComponentStaticCollisionShapeName>
		queryStaticEntity;

private:
	bool queueDestroy = false;
};

template <typename... Args>
void RealmServer::BroadcastReliable(const std::string &functionName,
									Args &&...args)
{
	Broadcast(icon7::FLAG_RELIABLE, functionName, std::forward<Args>(args)...);
}

template <typename... Args>
void RealmServer::BroadcastUnreliable(const std::string &functionName,
									  Args &&...args)
{
	Broadcast(icon7::FLAG_UNRELIABLE, functionName,
			  std::forward<Args>(args)...);
}

template <typename... Args>
void RealmServer::Broadcast(icon7::Flags flags, const std::string &functionName,
							Args &&...args)
{
	icon7::ByteBuffer buffer(1500);
	flags |= icon7::FLAGS_CALL_NO_FEEDBACK;
	rpc->SerializeSend(buffer, flags, functionName,
					   std::forward<Args>(args)...);
	Broadcast(buffer, 0);
}

template <typename... Args>
void RealmServer::BroadcastReliableExcept(uint64_t exceptEntityId,
										  const std::string &functionName,
										  Args &&...args)
{
	BroadcastExcept(exceptEntityId, icon7::FLAG_RELIABLE, functionName,
					std::forward<Args>(args)...);
}

template <typename... Args>
void RealmServer::BroadcastUnreliableExcept(uint64_t exceptEntityId,
											const std::string &functionName,
											Args &&...args)
{
	BroadcastExcept(exceptEntityId, icon7::FLAG_UNRELIABLE, functionName,
					std::forward<Args>(args)...);
}

template <typename... Args>
void RealmServer::BroadcastExcept(uint64_t exceptEntityId, icon7::Flags flags,
								  const std::string &functionName,
								  Args &&...args)
{
	icon7::ByteBuffer buffer(1500);
	flags |= icon7::FLAGS_CALL_NO_FEEDBACK;
	rpc->SerializeSend(buffer, flags, functionName,
					   std::forward<Args>(args)...);
	Broadcast(buffer, exceptEntityId);
}
