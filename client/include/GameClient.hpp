#pragma once

#include "../../ICon7/include/icon7/RPCEnvironment.hpp"
#include "../../ICon7/include/icon7/Peer.hpp"

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

	void Login(const std::string &username, const std::string &password);

	virtual void RunOneEpoch();

	virtual void RegisterObservers();

	bool IsConnected();
	bool IsDisconnected();

private: // rpc receiving methods
	void JoinRealm(const std::string &realm);
	void SpawnEntities(icon7::ByteReader *reader);
	void SpawnStaticEntities(icon7::ByteReader *reader);
	void UpdateEntities(icon7::ByteReader *reader);
	void SetModel(uint64_t serverId, ComponentModelName model,
				  ComponentShape shape);
	void DeleteEntities(icon7::ByteReader *reader);
	void SetPlayerEntityId(uint64_t serverId);
	void SetGravity(float gravity);
	void Pong(int64_t localTick, int64_t remoteTick);

	void SpawnEntity(uint64_t serverId,
					 const ComponentLastAuthoritativeMovementState state,
					 const ComponentName name, const ComponentModelName model,
					 const ComponentShape shape,
					 const ComponentMovementParameters movementParams);
	void SpawnStaticEntity(uint64_t serverId,
						   ComponentStaticTransform transform,
						   ComponentModelName model,
						   ComponentStaticCollisionShapeName shape);
	void UpdateEntity(uint64_t serverId,
					  const ComponentLastAuthoritativeMovementState state);
	void RemoveEntity(uint64_t serverId);

	void RequestSpawnOf(uint64_t serverId);

public: // game output api
	// may be called multiple times per single entity
	virtual void OnEnterRealm(const std::string &realmName) = 0;

	virtual void OnSetPlayerId(uint64_t localId) = 0;
	virtual void OnPlayerIdUnset() = 0;
	virtual void LoginFailed(std::string reason) = 0;
	virtual void LoginSuccessfull() = 0;

	virtual bool GetCollisionShape(std::string collisionShapeName,
								   TerrainCollisionData *data) = 0;

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

	void PerformSendPlayerMovementInput();

public:
	RealmClient *realm;

	icon7::RPCEnvironment rpc;
	icon7::Host *host;
	std::shared_ptr<icon7::Peer> peer;

	icon7::CommandExecutionQueue executionQueue;

	uint64_t localPlayerEntityId = 0;
	uint64_t serverPlayerEntityId = 0;
	std::string username;

	int64_t authdauthoritativePlayerSendDelay = 200;
	bool needSendPlayerMovementInput;
	int64_t lastTickAuthoritativeSent = 0;
	::Timer pingTimer;
	int64_t pingMs = 10;

	std::unordered_map<uint64_t, uint64_t> mapServerEntityIdToLocalEntityId;
	std::unordered_map<uint64_t, uint64_t> mapLocalEntityIdToServerEntityId;
};
