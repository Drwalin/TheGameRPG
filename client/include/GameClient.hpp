#pragma once

#include "../../ICon7/include/icon7/Forward.hpp"
#include "../../ICon7/include/icon7/CommandExecutionQueue.hpp"

#include "RealmClient.hpp"

class GameClient
{
public:
	GameClient();
	virtual ~GameClient();

	virtual void Destroy();

	void BindRpc();
	void RunNetworkLoopAsync();

	bool ConnectToServer(const std::string &ip, uint16_t port);
	void DisconnectRealmPeer();

	void Login(const std::string &username);

	virtual void RunOneEpoch();

	virtual void RegisterObservers();

	bool IsConnected();
	bool IsDisconnected();

private: // rpc receiving methods
	void JoinRealm(const std::string &realm, int64_t currentTick,
				   uint64_t playerEntityId);
	void SpawnEntities(icon7::ByteReader *reader);
	void SpawnStaticEntities(icon7::ByteReader *reader);
	void UpdateEntities(icon7::ByteReader *reader);
	void SetModel(uint64_t serverId, ComponentModelName model,
				  ComponentShape shape);
	void DeleteEntities(icon7::ByteReader *reader);
	void SetPlayerEntityId(uint64_t serverId);
	void SetGravity(float gravity);
	void Pong(int64_t clientLastSentTick, int64_t clientLastSentTickTimeNs,
			  int64_t serverLastProcessedTick,
			  int64_t serverTickStartTimeOffsetNs,
			  int64_t clientPingSentTimeNs);

	void SpawnEntity(uint64_t serverId,
					 const ComponentLastAuthoritativeMovementState &state,
					 const ComponentName &name, const ComponentModelName &model,
					 const ComponentShape &shape,
					 const ComponentMovementParameters &movementParams);
	void SpawnStaticEntity(uint64_t serverId,
						   ComponentStaticTransform transform,
						   ComponentModelName model,
						   ComponentCollisionShape shape);
	void UpdateEntity(uint64_t serverId,
					  const ComponentLastAuthoritativeMovementState state);
	void RemoveEntity(uint64_t serverId);

	void RequestSpawnOf(uint64_t serverId);
	void GenericComponentUpdate(icon7::ByteReader *reader);

	void
	PlayDeathAndDestroyEntity(uint64_t serverId, ComponentModelName modelName,
							  ComponentLastAuthoritativeMovementState state,
							  ComponentName name);
	void PlayAnimation(uint64_t serverId, ComponentModelName modelName,
					   ComponentLastAuthoritativeMovementState state,
					   std::string currentAnimation,
					   int64_t animationStartTick);

public: // rpc callbacks
	virtual void PlayDeathAndDestroyEntity_virtual(
		ComponentModelName modelName,
		ComponentLastAuthoritativeMovementState state, ComponentName name) = 0;
	virtual void
	PlayAnimation_virtual(uint64_t serverId, ComponentModelName modelName,
						  ComponentLastAuthoritativeMovementState state,
						  std::string currentAnimation,
						  int64_t animationStartTick) = 0;
	virtual void PlayFX(ComponentModelName modelName,
						ComponentStaticTransform transform,
						int64_t timeStartPlaying, uint64_t attachToEntityId,
						int32_t ttlMs) = 0;

public: // game output api
	// may be called multiple times per single entity
	virtual void OnEnterRealm(const std::string &realmName) = 0;

	virtual void OnSetPlayerId(uint64_t localId) = 0;
	virtual void OnPlayerIdUnset() = 0;
	virtual void LoginFailed(std::string reason) = 0;
	virtual void LoginSuccessfull() = 0;

public: // client input api
	void SetRotation(glm::vec3 rotation);
	void ProvideMovementInputDirection(glm::vec2 horizontalDirection);
	void TryPerformJump();
	glm::vec3 GetRotation();
	glm::vec3 GetPosition();
	glm::vec3 GetVelocity();
	ComponentShape GetShape();
	bool GetOnGround();
	bool IsInPlayerControl();
	void PerformInteractionUse();
	void PerformAttack(int64_t attackType, int64_t attackId, int64_t argInt);
	int64_t GetPing();
	int64_t GetCurrentTick();
	std::unordered_map<std::string, std::string> GetCharacteSheet();

	void PerformSendPlayerMovementInput();

	// Returns localId of hit entity
	uint64_t PerformRaytestFromEyes(ComponentMovementState state, float range,
									glm::vec3 *hitPos, glm::vec3 *normal,
									uint64_t *serverEntityId, uint32_t mask);

public:
	RealmClient *realm;

	icon7::RPCEnvironment *rpc;
	std::shared_ptr<icon7::Host> host;
	std::shared_ptr<icon7::Loop> loop;
	std::shared_ptr<icon7::Peer> peer;

	icon7::CommandExecutionQueue executionQueue;

	uint64_t localPlayerEntityId = 0;
	uint64_t serverPlayerEntityId = 0;
	std::string username;

	int64_t authdauthoritativePlayerSendDelay = 200;
	bool needSendPlayerMovementInput = false;
	int64_t lastTickAuthoritativeSent = 0;
	int64_t pingTickCounter = 100;
	int64_t pingMs = 70;

	std::unordered_map<uint64_t, uint64_t> mapServerEntityIdToLocalEntityId;
	std::unordered_map<uint64_t, uint64_t> mapLocalEntityIdToServerEntityId;
};
