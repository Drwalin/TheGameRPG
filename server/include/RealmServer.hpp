#pragma once

#include <vector>
#include <unordered_map>

#include <icon7/RPCEnvironment.hpp>
#include <icon7/Peer.hpp>

#include "../../common/include/EntityStorage.hpp"

#include "EntityServer.hpp"

#include "../../common/include/RealmBase.hpp"

class RealmServer : public RealmBase
{
public:
	RealmServer();
	virtual ~RealmServer() override;

	virtual void InitByRealmName(const std::string &realmName) override;

	void Update();
	void UpdateAllEntities();

	void AddPeer(icon7::Peer *peer);
	void DisconnectPeer(icon7::Peer *peer);

	void ExecuteOnRealmThread(icon7::Peer *peer, icon7::ByteReader *reader,
							  void (*function)(icon7::Peer *,
											   std::vector<uint8_t> &, void *));
	
	virtual EntityBase *GetEntity(uint64_t entityId) override;

protected:
	virtual uint64_t _InternalAddEntity() override;
	virtual uint64_t _InternalDestroyEntity(uint64_t entityId) override;

public:
	void BroadcastEntitiesMovementState();
	void BroadcastEntityChangeModel(uint64_t entityId);
	void BroadcastEntityLongState(uint64_t entityId);
	void BroadcastEntityErase(uint64_t entityId);

	void Broadcast(const std::vector<uint8_t> &buffer, icon7::Flags flags,
				   uint64_t exceptEntityId);

	template <typename... Args>
	void BroadcastReliable(const std::string &functionName, Args... args);
	
public:
	void RequestLongEntitiesData(icon7::Peer *peer, icon7::ByteReader *reader);

public:
	Timer sendEntitiesToClientsTimer;

	icon7::RPCEnvironment *rpc;
	icon7::CommandExecutionQueue executionQueue;

	std::unordered_set<icon7::Peer *> peers;
	EntityStorage<EntityServer> entityStorage;

	uint64_t sendUpdateDeltaTicks = 250;
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
