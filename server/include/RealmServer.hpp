#pragma once

#include <memory>

#include "../../ICon7/include/icon7/Flags.hpp"
#include "../../ICon7/include/icon7/Forward.hpp"
#include "../../ICon7/include/icon7/CommandExecutionQueue.hpp"

#include "PeerData.hpp"

#include "../../common/include/Realm.hpp"

class ServerCore;

class RealmServer : public Realm,
					public std::enable_shared_from_this<RealmServer>
{
public:
	inline constexpr static icon7::time::diff DEFAULT_TICK_DURATION = icon7::time::milliseconds(50);
	
	RealmServer();
	virtual ~RealmServer() override;

	void DisconnectAllAndDestroy();

	virtual void Init(const std::string &realmName) override;
	std::string GetReadFileName(bool *isFromSavedState);
	std::string GetWriteFileName();
	bool LoadFromFile();
	void SaveAllEntitiesToFiles();
	void SaveNonPlayerEntitiesToFile();
	void SavePlayerEntitiesToFiles();
	void SavePlayerEntityToFile(flecs::entity entity);

	void FlushSavingData();
	void WaitForFlushedData();

	void ConnectPeer(icon7::Peer *peer);
	void DisconnectPeer(icon7::Peer *peer);

	void ExecuteOnRealmThread(icon7::CommandHandle<icon7::Command> &&command);

	virtual void ExecuteMovementUpdate(uint64_t entityId,
									   ComponentMovementState *state) override;

	virtual uint64_t NewEntity() override;

	void RegisterObservers();
	void RegisterObservers_CharacterSheet();

	void QueueDestroy();
	bool IsQueuedToDestroy();

protected:
	// returns false if was not busy
	virtual void OneEpoch() override;

public: // Entity Actions
	/*
	 * argInt:
	 *    0 - for pressed attack button
	 *    1 - for released attack button
	 */
	void InteractInLineOfSight(uint64_t instigatorId,
							   ComponentLastAuthoritativeMovementState state,
							   uint64_t targetId, glm::vec3 dstPos,
							   glm::vec3 normal);
	void Attack(uint64_t instigatorId,
				ComponentLastAuthoritativeMovementState state,
				uint64_t targetId, glm::vec3 targetPos, int64_t attackType,
				int64_t attackId, int64_t argInt);

	void InteractInLineOfSight(icon7::Peer *peer,
							   ComponentLastAuthoritativeMovementState state,
							   uint64_t targetId, glm::vec3 dstPos,
							   glm::vec3 normal);
	void Attack(icon7::Peer *peer,
				ComponentLastAuthoritativeMovementState state,
				uint64_t targetId, glm::vec3 targetPos, int64_t attackType,
				int64_t attackId, int64_t argInt);

private:
	void StorePlayerDataInPeerAndFile(icon7::Peer *peer);
	void SetNextTickFotSavingData();

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

	Tick sendEntitiesToClientsTimer = {0};
	Tick sendUpdateDeltaTicks = {15};
	flecs::query<const ComponentLastAuthoritativeMovementState>
		queryLastAuthoritativeState;

	flecs::query<const ComponentLastAuthoritativeMovementState,
				 const ComponentName, const ComponentModelName,
				 const ComponentShape, const ComponentMovementParameters>
		queryEntityLongState;

	flecs::query<const ComponentStaticTransform, const ComponentModelName,
				 const ComponentCollisionShape>
		queryStaticEntity;

private:
	bool queueDestroy = false;

	Tick nextTickToSaveAllDataToFiles = {0};

public:
	bool currentlyUpdatingPlayerPeerEntityMovement = false;
};

#ifdef ENABLE_REALM_SERVER_IMPLEMENTATION_TEMPLATE
#include "../include/RealmServer.inl.hpp"
#endif
