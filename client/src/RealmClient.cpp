#include "../../ICon7/include/icon7/Debug.hpp"

#include "../include/GameClient.hpp"
#include "../include/EntityComponentsClient.hpp"

#include "../../common/include/EntitySystems.hpp"

#include "../include/RealmClient.hpp"

RealmClient::RealmClient(GameClient *gameClient) : gameClient(gameClient)
{
	RealmClient::RegisterObservers();
}

RealmClient::~RealmClient() {}

void RealmClient::Init(const std::string &realmName)
{
	// TODO: load static realm data from database/disk
	Realm::Init(realmName);
}

void RealmClient::Clear()
{
	auto ar = gameClient->mapLocalEntityIdToServerEntityId;
	for (auto it = ar.begin(); it != ar.end(); ++it) {
		flecs::entity entity = Entity(it->first);
		if (entity.is_valid() && entity.is_alive()) {
			entity.destruct();
		}
	}
	gameClient->mapLocalEntityIdToServerEntityId.clear();
	gameClient->mapServerEntityIdToLocalEntityId.clear();

	Realm::Clear();
}

void RealmClient::Reinit(const std::string &realmName)
{
	Clear();
	Init(realmName);
	gameClient->OnEnterRealm(realmName);
}

void RealmClient::OneEpoch() { Realm::OneEpoch(); }

void RealmClient::AddNewAuthoritativeMovementState(
	uint64_t localId, uint64_t serverId, ComponentMovementState _state,
	Tick tick)
{
	ComponentMovementState state = _state;
	ComponentMovementHistory *movement =
		AccessComponent<ComponentMovementHistory>(localId);
	auto &states = movement->states;
	int i = states.size() - 1;
	for (; i >= 0; --i) {
		if (states[i].tick < tick) {
			break;
		} else if (states[i].tick == tick) {
			states[i] = {state, tick};
			return;
		}
	}
	states.insert(states.begin() + i + 1, {state, tick});
}

void RealmClient::UpdateEntityCurrentState(uint64_t localId, uint64_t serverId)
{
	ComponentMovementState state = ExecuteMovementUpdate(localId);
	flecs::entity entity = Entity(localId);
	entity.set<ComponentMovementState>(state);
}

ComponentMovementState RealmClient::ExecuteMovementUpdate(uint64_t entityId)
{
	auto entity = Entity(entityId);

	const ComponentShape *shape = entity.try_get<ComponentShape>();
	if (shape == nullptr) {
		LOG_ERROR("Character %lu does not have shape component", entityId);
		return {};
	}
	ComponentMovementState *_currentState =
		entity.try_get_mut<ComponentMovementState>();
	if (_currentState == nullptr) {
		LOG_ERROR("Character %lu does not have ComponentMovementState",
				  entityId);
		return {};
	}
	ComponentMovementState state = *_currentState;
	const ComponentMovementParameters *movementParams =
		entity.try_get<ComponentMovementParameters>();
	if (movementParams == nullptr) {
		LOG_ERROR("Character %lu does not have ComponentMovementParameters",
				  entityId);
		return {};
	}

	ComponentMovementHistory *_states =
		entity.try_get_mut<ComponentMovementHistory>();
	if (entityId != gameClient->localPlayerEntityId && _states != nullptr &&
		_states->states.size() > 0) {
		auto &states = _states->states;

		Tick currentTick = timer.currentTick;
		
		const float _t = timer.GetFactorToNextTick(tickDuration);
		currentTick += glm::floor(_t);
		const float t = _t - glm::floor(_t);

		int id = states.size() - 1;
		for (; id >= 0; --id) {
			if (states[id].tick <= currentTick) {
				break;
			}
		}

		if (id >= 0) {
			if (id + 1 < states.size()) {
				ComponentMovementState prev;
				Tick prevTick;
				if (id >= 0) {
					prev = states[id].state__;
					prevTick = states[id].tick;
				} else {
					prev = state;
					prevTick = currentTick;
				}
				ComponentMovementState next = states[id + 1].state__;

				const glm::vec3 A = prev.pos;
				const glm::vec3 B = next.pos;
				const Tick nextTick = states[id+1].tick;
				
				const float currentFloatTIck = timer.currentTick.v + t;
				
				const float f = (currentFloatTIck - prevTick.v) / (nextTick - prevTick).v;

// 				const float dt = t * Realm::TICK_DURATION_SECONDS;

// 				printf("%10.3f s:   %7.3f ->   %.3f     factor: %.3f", timer.GetCurrentTimepoint().sec(), currentFloatTIck, t, f);
// 				printf("         current tick: %li        last two ticks: %li %li \n", currentTick.v, states[id].tick.v, states[id+1].tick.v);

				
// 				const glm::vec3 p1 = A + prev.vel * dt;
				const glm::vec3 p2 = A * (1.0f - f) + B * f;
				state.vel = prev.vel * (1.0f - f) + next.vel * f;

// 				const glm::vec3 P = p1 * (1.0f - f) + p2 * f;
				const glm::vec3 P = p2;

				state.pos = P;
				state.rot = prev.rot * (1 - f) + next.rot * f;
				state.onGround = next.onGround;
			} else {
				if (states.size() >= 2) {
					LOG_TRACE("current tick: %li        last two ticks: %li %li", currentTick.v, states[states.size()-2].tick.v, states.back().tick.v);
				}
				state = states.back().state__;
			}
		} else {
			LOG_TRACE("");
			state = states[0].state__;
		}
		if (id > 50) {
			LOG_TRACE("");
			states.erase(states.begin(), states.begin() + id - 3);
		}
		return state;
	} else {
		LOG_TRACE("");
		EntitySystems::UpdateMovement(this, entity, *shape, state, state,
									  *movementParams,
									  timer.GetFactorToNextTick(tickDuration),
									  false);
	}
	return state;
}

void RealmClient::RegisterObservers()
{
	RegisterObserver(
		flecs::OnAdd, +[](flecs::entity entity, const ComponentMovementState &,
						  const ComponentMovementState &, const ComponentName,
						  const ComponentModelName, const ComponentShape,
						  const ComponentName &name) {
			entity.add<ComponentMovementHistory>();
		});

	RegisterObserver(flecs::OnRemove, [this](flecs::entity entity,
											 const ComponentMovementState &) {
		entity.destruct();
		auto it =
			gameClient->mapLocalEntityIdToServerEntityId.find(entity.id());
		if (it != gameClient->mapLocalEntityIdToServerEntityId.end()) {
			gameClient->mapServerEntityIdToLocalEntityId.erase(it->second);
			gameClient->mapLocalEntityIdToServerEntityId.erase(it);
		}
	});

	RegisterObserver(flecs::OnRemove, [this](flecs::entity entity,
											 const ComponentStaticTransform &) {
		entity.destruct();
		auto it =
			gameClient->mapLocalEntityIdToServerEntityId.find(entity.id());
		if (it != gameClient->mapLocalEntityIdToServerEntityId.end()) {
			gameClient->mapServerEntityIdToLocalEntityId.erase(it->second);
			gameClient->mapLocalEntityIdToServerEntityId.erase(it);
		}
	});
}
