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
	uint64_t localId, uint64_t serverId,
	ComponentMovementState _state, Tick tick)
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
	ComponentMovementState state;
	ExecuteMovementUpdate(localId, &state);
	flecs::entity entity = Entity(localId);
	entity.set<ComponentMovementState>(state);
}

void RealmClient::ExecuteMovementUpdate(uint64_t entityId,
										ComponentMovementState *state)
{
	auto entity = Entity(entityId);

	const ComponentShape *shape = entity.try_get<ComponentShape>();
	if (shape == nullptr) {
		LOG_ERROR("Character %lu does not have shape component", entityId);
		return;
	}
	ComponentMovementState *currentState =
		entity.try_get_mut<ComponentMovementState>();
	if (currentState == nullptr) {
		LOG_ERROR("Character %lu does not have ComponentMovementState", entityId);
		return;
	}
	const ComponentMovementParameters *movementParams =
		entity.try_get<ComponentMovementParameters>();
	if (movementParams == nullptr) {
		LOG_ERROR("Character %lu does not have ComponentMovementParameters", entityId);
		return;
	}

	ComponentMovementHistory *_states =
		entity.try_get_mut<ComponentMovementHistory>();
	if (entityId != gameClient->localPlayerEntityId && _states != nullptr &&
		_states->states.size() > 0) {
		auto &states = _states->states;

		Tick currentTick = timer.currentTick;

		int id = states.size() - 1;
		for (; id >= 0; --id) {
			if (states[id].tick <= currentTick) {
				break;
			}
		}

		if (id >= 0) {
			if (*currentState != states[id].state__) {
				*currentState = states[id].state__;
			}

			if (id + 1 < states.size()) {
				ComponentMovementState prev;
				if (id >= 0) {
					prev = states[id].state__;
				} else {
					prev = *currentState;
				}
				ComponentMovementState next = states[id + 1].state__;

				{
					EntitySystems::UpdateMovement(this, entity, *shape,
												  *currentState, {prev},
												  *movementParams);
				}

				const glm::vec3 A = prev.pos;
				const glm::vec3 B = next.pos;
				const glm::vec3 V = prev.vel;
				const float fullDt = Realm::TICK_DURATION_SECONDS;

				const glm::vec3 a = (B - A - V * fullDt) / (fullDt * fullDt) * 2.0f;
				const float t = timer.GetFactorToNextTick(tickDuration);
				const float dt = t * Realm::TICK_DURATION_SECONDS;

				currentState->vel = V + a * dt;
				glm::vec3 P = A + V * dt + a * dt * dt * 0.5f;
				currentState->pos = currentState->pos * (1 - t) + P * t;
				currentState->rot = prev.rot * (1 - t) + next.rot * t;
				currentState->onGround = next.onGround;
			} else {
				*currentState = states.back().state__;
			}

			auto las = *currentState;

			EntitySystems::UpdateMovement(this, entity, *shape, *currentState,
										  las, *movementParams);
		} else {
			*state = states[0].state__;
		}
		if (id > 10) {
			states.erase(states.begin(), states.begin() + id - 3);
		}
	} else {
		EntitySystems::UpdateMovement(this, entity, *shape, *currentState,
									  *currentState, *movementParams);
	}
	*state = *currentState;
}

void RealmClient::RegisterObservers()
{
	RegisterObserver(
		flecs::OnAdd, +[](flecs::entity entity, const ComponentMovementState &,
						  const ComponentMovementState &,
						  const ComponentName, const ComponentModelName,
						  const ComponentShape, const ComponentName &name) {
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
