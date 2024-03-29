#pragma once

#include <icon7/RPCEnvironment.hpp>
#include <icon7/Peer.hpp>

#include "RealmClient.hpp"

class ClientCore
{
public:
	ClientCore();
	virtual ~ClientCore();
	
	void SetUsername();

	void RunNetworkLoopSync();
	void RunNetworkLoopAsync();

	void BindRpc();

	bool ConnectToRealmServer(const std::string &ip, uint16_t port);
	void DisconnectRealmPeer();

	void RunOneEpoch();

public: // player output api
	virtual void OnEntityAdd(uint64_t entityId) = 0;
	virtual void OnEntityRemove(uint64_t entityId) = 0;
	virtual void OnEntityShape(uint64_t entityId, const EntityShape &shape) = 0;
	virtual void OnEntityModel(uint64_t entityId,
							   const EntityModelName &model) = 0;
	virtual void
	OnEntityCurrentMovementStateUpdate(uint64_t entityId,
									 const EntityMovementState &state) = 0;
	
	virtual void OnRealmNames(std::vector<std::string> realmNames);
	
public: // player input api

	void ProvideMovementInputDirection(glm::vec2 horizontalDirection);
	void TryPerformJump();
	
	void SetUsername(std::string username);
	
public:
	RealmClient realm;

	icon7::RPCEnvironment rpc;
	icon7::Host *host;
	std::shared_ptr<icon7::Peer> realmConnectionPeer;

	icon7::CommandExecutionQueue executionQueue;
	
	uint64_t playerEntityId;
	std::string username;
};
