#include <chrono>
#include <thread>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/mat3x3.hpp>
#include <glm/gtx/quaternion.hpp>

#include "../../ICon7/include/icon7/PeerUStcp.hpp"
#include "../../ICon7/include/icon7/HostUStcp.hpp"
#include "../../ICon7/include/icon7/Command.hpp"

#include "../include/ServerRpcProxy.hpp"

#include "../include/GameClient.hpp"

GameClient::GameClient() : realm(new RealmClient(this))
{
	icon7::uS::tcp::Host *_host = new icon7::uS::tcp::Host();
	_host->Init();
	host = _host;
	host->SetRpcEnvironment(&rpc);
	host->userPointer = this;

	// host->SetOnDisconnect();
	pingTimer.Start();
	lastTickAuthoritativeSent = 0;

	RegisterObservers();
}

void GameClient::RegisterObservers()
{
	realm->RegisterObserver(
		flecs::OnRemove,
		[this](flecs::entity entity, const ComponentMovementState &) {
			if (entity.id() == localPlayerEntityId) {
				OnPlayerIdUnset();
				serverPlayerEntityId = 0;
				localPlayerEntityId = 0;
			}
			auto it = mapLocalEntityIdToServerEntityId.find(entity.id());
			if (it != mapLocalEntityIdToServerEntityId.end()) {
				mapServerEntityIdToLocalEntityId.erase(it->second);
				mapLocalEntityIdToServerEntityId.erase(it);
			}
		});
}

GameClient::~GameClient() {}

bool GameClient::IsConnected()
{
	if (peer != nullptr) {
		return peer->IsReadyToUse();
	}
	return false;
}

bool GameClient::IsDisconnected()
{
	if (peer != nullptr) {
		return peer->IsDisconnecting() || peer->IsClosed();
	}
	return true;
}

void GameClient::Destroy()
{
	if (host) {
		peer = nullptr;
		host->DisconnectAllAsync();
		host->StopListening();
		host->WaitStopRunning();
		delete host;
		host = nullptr;
	}
	if (realm) {
		realm->Destroy();
		delete realm;
		realm = nullptr;
	}
}

void GameClient::RunNetworkLoopAsync() { host->RunAsync(); }

void GameClient::DisconnectRealmPeer()
{
	realm->Clear();
	if (serverPlayerEntityId || localPlayerEntityId) {
		OnPlayerIdUnset();
		serverPlayerEntityId = 0;
		localPlayerEntityId = 0;
	}
	if (peer &&
		(peer->IsClosed() == false || peer->IsDisconnecting() == false)) {
		peer->Disconnect();
	}
	realm->Clear();
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

	peer = state->sp;
	return true;
}

void GameClient::RunOneEpoch()
{
	executionQueue.Execute(128);

	PerformSendPlayerMovementInput();
	realm->OneEpoch();
	PerformSendPlayerMovementInput();

	pingTimer.Update();
	if (abs(lastPingTime - pingTimer.currentTick) > 8000) {
		ServerRpcProxy::Ping(this, true);
		lastPingTime = pingTimer.currentTick;
	}
}

void GameClient::PerformSendPlayerMovementInput()
{
	if (localPlayerEntityId == 0) {
		return;
	}
	if (lastTickAuthoritativeSent + authdauthoritativePlayerSendDelay >
			realm->timer.currentTick &&
		needSendPlayerMovementInput == false) {
		return;
	}

	lastTickAuthoritativeSent = realm->timer.currentTick;
	auto state =
		realm->GetComponent<ComponentMovementState>(localPlayerEntityId);
	ServerRpcProxy::UpdatePlayer(this, *state);

	needSendPlayerMovementInput = false;
}

glm::vec3 GameClient::GetRotation()
{
	if (localPlayerEntityId == 0) {
		// TODO: maybe error?
		return {0, 0, 0};
	}
	flecs::entity player = realm->Entity(localPlayerEntityId);
	auto oldState = player.get<ComponentMovementState>();
	if (oldState)
		return oldState->rot;
	LOG_TRACE("ERROR");
	return {0, 0, 0};
}

glm::vec3 GameClient::GetPosition()
{
	if (localPlayerEntityId == 0) {
		// TODO: maybe error?
		return {0, 0, 0};
	}
	flecs::entity player = realm->Entity(localPlayerEntityId);
	auto oldState = player.get<ComponentMovementState>();
	if (oldState)
		return oldState->pos;
	LOG_TRACE("ERROR");
	return {0, 0, 0};
}

glm::vec3 GameClient::GetVelocity()
{
	if (localPlayerEntityId == 0) {
		// TODO: maybe error?
		return {0, 0, 0};
	}
	flecs::entity player = realm->Entity(localPlayerEntityId);
	auto oldState = player.get<ComponentMovementState>();
	if (oldState)
		return oldState->vel;
	LOG_TRACE("ERROR");
	return {0, 0, 0};
}

bool GameClient::GetOnGround()
{
	if (localPlayerEntityId == 0) {
		// TODO: maybe error?
		return true;
	}
	flecs::entity player = realm->Entity(localPlayerEntityId);
	auto oldState = player.get<ComponentMovementState>();
	if (oldState)
		return oldState->onGround;
	LOG_TRACE("ERROR");
	return true;
}

ComponentShape GameClient::GetShape()
{
	if (localPlayerEntityId == 0) {
		// TODO: maybe error?
		return {0, 0};
	}
	flecs::entity player = realm->Entity(localPlayerEntityId);
	auto shape = player.get<ComponentShape>();
	if (shape)
		return *shape;
	LOG_TRACE("ERROR");
	return {0, 0};
}

bool GameClient::IsInPlayerControl()
{
	return localPlayerEntityId != 0 && serverPlayerEntityId != 0;
}

void GameClient::SetRotation(glm::vec3 rotation)
{
	if (localPlayerEntityId == 0) {
		// TODO: maybe error?
		return;
	}
	flecs::entity player = realm->Entity(localPlayerEntityId);
	auto oldState = player.get<ComponentMovementState>();
	if (oldState) {
		auto state = *oldState;
		state.rot = rotation;
		player.set(state);
		needSendPlayerMovementInput = true;
	} else {
		LOG_TRACE("ERROR");
	}
}

void GameClient::ProvideMovementInputDirection(glm::vec2 horizontalDirection)
{
	if (localPlayerEntityId == 0) {
		// TODO: maybe error?
		return;
	}
	flecs::entity player = realm->Entity(localPlayerEntityId);
	auto stateP = player.get<ComponentMovementState>();
	if (stateP == nullptr) {
		LOG_TRACE("ERROR");
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
	vel *=
		player.get<ComponentMovementParameters>()->maxMovementSpeedHorizontal;

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
	flecs::entity player = realm->Entity(localPlayerEntityId);
	auto stateP = player.get<ComponentMovementState>();
	if (stateP == nullptr) {
		LOG_TRACE("ERROR");
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

void GameClient::PerformInteractionUse()
{
	if (localPlayerEntityId == 0) {
		// TODO: maybe error?
		return;
	}
	flecs::entity player = realm->Entity(localPlayerEntityId);
	auto state = player.get<ComponentMovementState>();
	if (state == nullptr) {
		// TODO: maybe error?
		return;
	}
	auto characterSheet = player.get<ComponentCharacterSheet>();
	if (characterSheet == nullptr) {
		// TODO: maybe error?
		return;
	}

	glm::vec3 hitPoint, normal;
	bool hasNormal;
	uint64_t serverEntityId = 0;
	uint64_t localTargetId =
		PerformRaytestFromEyes(*state, characterSheet->useRange, &hitPoint,
							   &normal, &hasNormal, &serverEntityId);
	if (localTargetId) {
		ServerRpcProxy::InteractInLineOfSight(this, *state, serverEntityId,
											  hitPoint, normal);
	}
}

void GameClient::PerformAttack(int64_t attackType, int64_t attackId,
							   const std::string &argStr, int64_t argInt)
{
	if (localPlayerEntityId == 0) {
		// TODO: maybe error?
		return;
	}
	flecs::entity player = realm->Entity(localPlayerEntityId);
	auto state = player.get<ComponentMovementState>();
	if (state == nullptr) {
		// TODO: maybe error?
		return;
	}

	glm::vec3 hitPoint, normal;
	bool hasNormal;
	uint64_t serverEntityId = 0;
	uint64_t localTargetId = PerformRaytestFromEyes(
		*state, 1000.0f, &hitPoint, &normal, &hasNormal, &serverEntityId);
	if (localTargetId) {
		ServerRpcProxy::Attack(this, *state, serverEntityId, hitPoint,
							   attackType, attackId, argStr, argInt);
	} else {
		ServerRpcProxy::Attack(this, *state, 0, hitPoint, attackType, attackId,
							   argStr, argInt);
	}
}

uint64_t GameClient::PerformRaytestFromEyes(ComponentMovementState state,
											float range, glm::vec3 *hitPos,
											glm::vec3 *normal, bool *hasNormal,
											uint64_t *serverEntityId)
{
	*serverEntityId = 0;
	*hasNormal = false;
	if (localPlayerEntityId == 0) {
		// TODO: maybe error?
		return 0;
	}

	glm::vec3 pos = GetPosition();
	glm::vec3 rot = GetRotation();

	glm::mat4 mat(1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1);
	mat = glm::rotate(mat, rot.y, glm::vec3(0, 1, 0));
	mat = glm::rotate(mat, rot.x, glm::vec3(-1, 0, 0));
	glm::vec3 forward = mat * glm::vec4(0, 0, range, 0);

	pos.y += GetShape().height;

	glm::vec3 dst = pos + forward;

	float td;
	uint64_t entityId = 0;
	if (realm->collisionWorld.RayTestFirstHit(pos, dst, hitPos, normal,
											  &entityId, &td, hasNormal,
											  localPlayerEntityId)) {
		if (entityId != 0) {
			auto it = mapLocalEntityIdToServerEntityId.find(entityId);
			if (it != mapLocalEntityIdToServerEntityId.end()) {
				*serverEntityId = it->second;
			}
		}
	}
	return entityId;
}

int64_t GameClient::GetPing() { return pingMs; }

int64_t GameClient::GetCurrentTick() { return realm->timer.currentTick; }
