#include <chrono>

#include <icon7/PeerUStcp.hpp>
#include <icon7/HostUStcp.hpp>
#include <icon7/Command.hpp>
#include <icon7/Flags.hpp>

#include "../include/ServerRpcProxy.hpp"

#include "../include/GameClient.hpp"

GameClient::GameClient()
{
	host = nullptr;
	pingTimer.Start();
}

GameClient::~GameClient()
{
	host->DisconnectAllAsync();
	host->StopListening();
	host->WaitStopRunning();
	delete host;
	host = nullptr;
}

void GameClient::RunNetworkLoopAsync() { host->RunAsync(); }

void GameClient::DisconnectRealmPeer() { realmConnectionPeer->Disconnect(); }

bool GameClient::ConnectToServer(const std::string &ip, uint16_t port)
{
	DisconnectRealmPeer();

	std::future<std::shared_ptr<icon7::Peer>> peerFuture =
		host->ConnectPromise(ip, port);
	peerFuture.wait_for(std::chrono::seconds(5));
	if (peerFuture.valid() == false) {
		return false;
	}
	realmConnectionPeer = peerFuture.get();

	ServerRpcProxy::Ping(this, true);

	return true;
}

void GameClient::RunOneEpoch()
{
	PerformSendPlayerMovementInput();
	realm.OneEpoch();
	PerformSendPlayerMovementInput();
}

void GameClient::PerformSendPlayerMovementInput()
{
	if (needSendPlayerMovementInput == false) {
		return;
	}
	if (localPlayerEntityId == 0) {
		return;
	}

	auto state = realm.GetComponent<EntityLastAuthoritativeMovementState>(
		localPlayerEntityId);
	ServerRpcProxy::UpdatePlayer(this, *state);

	needSendPlayerMovementInput = false;
}

void GameClient::SetRotation(glm::vec3 rotation)
{
	if (localPlayerEntityId == 0) {
		// TODO: maybe error?
		return;
	}
	flecs::entity player = realm.Entity(localPlayerEntityId);
	auto oldState = *player.get<EntityLastAuthoritativeMovementState>();
	oldState.oldState.rot = rotation;
	player.set(oldState);
	needSendPlayerMovementInput = true;
}

void GameClient::ProvideMovementInputDirection(glm::vec2 horizontalDirection)
{
	if (localPlayerEntityId == 0) {
		// TODO: maybe error?
		return;
	}
	flecs::entity player = realm.Entity(localPlayerEntityId);
	auto oldState = *player.get<EntityLastAuthoritativeMovementState>();
	if (oldState.oldState.onGround == false) {
		// TODO: no air control implemented
		return;
	}

	glm::vec3 vel;
	vel.x = horizontalDirection.x;
	vel.z = horizontalDirection.y;
	vel.y = 0;
	vel *= player.get<EntityMovementParameters>()->maxMovementSpeedHorizontal;

	oldState.oldState.vel = vel;

	player.set(oldState);
	needSendPlayerMovementInput = true;
}

void GameClient::TryPerformJump()
{
	if (localPlayerEntityId == 0) {
		// TODO: maybe error?
		return;
	}
	flecs::entity player = realm.Entity(localPlayerEntityId);
	auto oldState = *player.get<EntityLastAuthoritativeMovementState>();
	if (oldState.oldState.onGround == false) {
		// TODO: no air jumps
		return;
	}

	glm::vec3 vel = oldState.oldState.vel;
	// TODO: use some true jump velocity
	vel.y = 5;

	oldState.oldState.vel = vel;

	player.set(oldState);
	needSendPlayerMovementInput = true;
}
