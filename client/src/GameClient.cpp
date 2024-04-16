#include <chrono>
#include <thread>

#include <icon7/PeerUStcp.hpp>
#include <icon7/HostUStcp.hpp>
#include <icon7/Command.hpp>
#include <icon7/Flags.hpp>

#include "../include/ServerRpcProxy.hpp"

#include "../include/GameClient.hpp"

GameClient::GameClient() : realm(this)
{
	icon7::uS::tcp::Host *_host = new icon7::uS::tcp::Host();
	_host->Init();
	host = _host;
	host->SetRpcEnvironment(&rpc);
	host->userPointer = this;

	// host->SetOnDisconnect();
	pingTimer.Start();
}

GameClient::~GameClient()
{
	if (host) {
		host->DisconnectAllAsync();
		host->StopListening();
		host->WaitStopRunning();
		delete host;
		host = nullptr;
	}
}

void GameClient::RunNetworkLoopAsync() { host->RunAsync(); }

void GameClient::DisconnectRealmPeer()
{
	if (realmConnectionPeer) {
		realmConnectionPeer->Disconnect();
	}
}

bool GameClient::ConnectToServer(const std::string &ip, uint16_t port)
{
	DisconnectRealmPeer();

	icon7::commands::ExecuteOnPeer com{};

	struct X {
		std::shared_ptr<icon7::Peer> sp;
		std::atomic<icon7::Peer *> rp;
		std::atomic<int> s;
	};

	using _T = std::shared_ptr<X>;
	_T state(new X{nullptr, nullptr, 0});

	com.data.resize(sizeof(_T));
	new (com.data.data()) _T(state);
	com.function = [](auto peer, auto bytes, auto ptr) {
		_T *v = (_T *)bytes.data();
		if (peer) {
			(*v)->sp = peer->shared_from_this();
			(*v)->rp.store(peer);
			(*v)->s.store(1);
		} else {
			(*v)->s.store(-1);
		}
		v->~shared_ptr();
	};

	host->Connect(ip, port, std::move(com));

	auto end = std::chrono::steady_clock::now() + std::chrono::seconds(5);
	while (end > std::chrono::steady_clock::now()) {
		if (state->s != 0) {
			break;
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(10));
	}
	if (state->rp.load() == nullptr) {
		return false;
	}

	realmConnectionPeer = state->sp;
	ServerRpcProxy::Ping(this, true);
	return true;

	// 	std::future<std::shared_ptr<icon7::Peer>> peerFuture =
	// 		host->ConnectPromise(ip, port);
	// 	std::this_thread::sleep_for(std::chrono::seconds(5));
	// 	peerFuture.wait_for(std::chrono::seconds(5));
	// 	if (peerFuture.valid() == false) {
	// 		return false;
	// 	}
	// 	realmConnectionPeer = peerFuture.get();
	//
	// 	ServerRpcProxy::Ping(this, true);
	//
	// 	return true;
}

void GameClient::RunOneEpoch()
{
	const uint32_t MAX_EVENTS = 128;
	icon7::Command commands[MAX_EVENTS];
	const uint32_t dequeued =
		executionQueue.TryDequeueBulkAny(commands, MAX_EVENTS);

	for (uint32_t i = 0; i < dequeued; ++i) {
		commands[i].Execute();
		commands[i].~Command();
	}

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

glm::vec3 GameClient::GetRotation()
{
	if (localPlayerEntityId == 0) {
		// TODO: maybe error?
		return {0,0,0};
	}
	flecs::entity player = realm.Entity(localPlayerEntityId);
	auto oldState = *player.get<EntityLastAuthoritativeMovementState>();
	return oldState.oldState.rot;
}

glm::vec3 GameClient::GetPosition()
{
	if (localPlayerEntityId == 0) {
		// TODO: maybe error?
		return {0, 0, 0};
	}
	flecs::entity player = realm.Entity(localPlayerEntityId);
	auto oldState = *player.get<EntityLastAuthoritativeMovementState>();
	return oldState.oldState.pos;
}

glm::vec3 GameClient::GetVelocity()
{
	if (localPlayerEntityId == 0) {
		// TODO: maybe error?
		return {0, 0, 0};
	}
	flecs::entity player = realm.Entity(localPlayerEntityId);
	auto oldState = *player.get<EntityLastAuthoritativeMovementState>();
	return oldState.oldState.vel;
}

EntityShape GameClient::GetShape()
{
	if (localPlayerEntityId == 0) {
		// TODO: maybe error?
		return {0, 0};
	}
	flecs::entity player = realm.Entity(localPlayerEntityId);
	auto shape = *player.get<EntityShape>();
	return shape;
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
// 	if (oldState.oldState.onGround == false) {
// 		DEBUG("Trying to move in the air");
// 		// TODO: no air control implemented
// 		return;
// 	}

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
