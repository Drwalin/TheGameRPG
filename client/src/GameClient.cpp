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
	lastTickAuthoritativeSent = 0;
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
		realmConnectionPeer = nullptr;
	}
}

bool GameClient::ConnectToServer(const std::string &ip, uint16_t port)
{
	DisconnectRealmPeer();

	struct X {
		std::shared_ptr<icon7::Peer> sp;
		std::atomic<icon7::Peer *> rp = nullptr;
		std::atomic<int> s = 0;
	};

	class CommandConnectToServer : public icon7::commands::ExecuteOnPeer
	{
	public:
		CommandConnectToServer() = default;
		~CommandConnectToServer() = default;
		std::shared_ptr<X> v;
		virtual void Execute() override
		{
			if (peer) {
				v->sp = peer->shared_from_this();
				v->rp.store(peer.get());
				v->s.store(1);
			} else {
				v->s.store(-1);
			}
		}
	};
	std::shared_ptr<X> state = std::make_shared<X>();
	auto com = icon7::CommandHandle<CommandConnectToServer>::Create();
	com->v = state;

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
	executionQueue.Execute(128);

	PerformSendPlayerMovementInput();
	realm.OneEpoch();
	// 	UpdatePlayerAuthoritativeState();
	PerformSendPlayerMovementInput();
}

void GameClient::UpdatePlayerAuthoritativeState()
{
	// TODO: is this function needed??
	if (localPlayerEntityId == 0) {
		return;
	}
	flecs::entity playerEntity = realm.Entity(localPlayerEntityId);
	auto state = playerEntity.get<EntityMovementState>();
	if (state) {
		playerEntity.set<EntityLastAuthoritativeMovementState>({*state});
	}
}

void GameClient::PerformSendPlayerMovementInput()
{
	if (localPlayerEntityId == 0) {
		return;
	}
	if (lastTickAuthoritativeSent + authdauthoritativePlayerSendDelay >
			realm.timer.currentTick &&
		needSendPlayerMovementInput == false) {
		return;
	}

	lastTickAuthoritativeSent = realm.timer.currentTick;
	auto state = realm.GetComponent<EntityMovementState>(localPlayerEntityId);
	ServerRpcProxy::UpdatePlayer(this, *state);

	needSendPlayerMovementInput = false;
}

glm::vec3 GameClient::GetRotation()
{
	if (localPlayerEntityId == 0) {
		// TODO: maybe error?
		return {0, 0, 0};
	}
	flecs::entity player = realm.Entity(localPlayerEntityId);
	auto oldState = player.get<EntityMovementState>();
	if (oldState)
		return oldState->rot;
	return {0, 0, 0};
}

glm::vec3 GameClient::GetPosition()
{
	if (localPlayerEntityId == 0) {
		// TODO: maybe error?
		return {0, 0, 0};
	}
	flecs::entity player = realm.Entity(localPlayerEntityId);
	auto oldState = player.get<EntityMovementState>();
	if (oldState)
		return oldState->pos;
	return {0, 0, 0};
}

glm::vec3 GameClient::GetVelocity()
{
	if (localPlayerEntityId == 0) {
		// TODO: maybe error?
		return {0, 0, 0};
	}
	flecs::entity player = realm.Entity(localPlayerEntityId);
	auto oldState = player.get<EntityMovementState>();
	if (oldState)
		return oldState->vel;
	return {0, 0, 0};
}

EntityShape GameClient::GetShape()
{
	if (localPlayerEntityId == 0) {
		// TODO: maybe error?
		return {0, 0};
	}
	flecs::entity player = realm.Entity(localPlayerEntityId);
	auto shape = player.get<EntityShape>();
	if (shape)
		return *shape;
	return {0, 0};
}

void GameClient::SetRotation(glm::vec3 rotation)
{
	if (localPlayerEntityId == 0) {
		// TODO: maybe error?
		return;
	}
	flecs::entity player = realm.Entity(localPlayerEntityId);
	auto oldState = player.get<EntityMovementState>();
	if (oldState) {
		auto state = *oldState;
		state.rot = rotation;
		player.set(state);
		needSendPlayerMovementInput = true;
	}
}

void GameClient::ProvideMovementInputDirection(glm::vec2 horizontalDirection)
{
	if (localPlayerEntityId == 0) {
		// TODO: maybe error?
		return;
	}
	flecs::entity player = realm.Entity(localPlayerEntityId);
	auto stateP = player.get<EntityMovementState>();
	if (stateP == nullptr) {
		return;
	}
	auto state = *stateP;
	if (state.onGround == false) {
		// TODO: no air control implemented
		return;
	}
	glm::vec3 oldVel = state.vel;

	glm::vec3 vel;
	vel.x = horizontalDirection.x;
	vel.z = horizontalDirection.y;
	vel *= player.get<EntityMovementParameters>()->maxMovementSpeedHorizontal;

	glm::vec3 dv = oldVel - vel;
	dv.y = 0;
	float d = fabs(dv.x) + fabs(dv.z);
	if (d > 0.001) {
		vel.y = state.vel.y;

		state.vel = vel;

		player.set(state);
		needSendPlayerMovementInput = true;
	}
}

void GameClient::TryPerformJump()
{
	if (localPlayerEntityId == 0) {
		// TODO: maybe error?
		return;
	}
	flecs::entity player = realm.Entity(localPlayerEntityId);
	auto stateP = player.get<EntityMovementState>();
	if (stateP == nullptr) {
		return;
	}
	auto state = *stateP;
	if (state.onGround == false) {
		// TODO: no air jumps
		return;
	}

	glm::vec3 vel = state.vel;
	// TODO: use some true jump velocity
	vel.y = 5.0f;

	state.vel = vel;

	player.set(state);
	needSendPlayerMovementInput = true;
}
