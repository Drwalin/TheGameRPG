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
	ComponentLastAuthoritativeMovementState _state)
{
	_state.oldState.timestamp += TICKS_UPDATE_DELAY;
	ComponentMovementState state = _state.oldState;
	ComponentMovementHistory *movement =
		AccessComponent<ComponentMovementHistory>(localId);
	auto &states = movement->states;
	int i = states.size() - 1;
	for (; i >= 0; --i) {
		if (states[i].timestamp < state.timestamp) {
			break;
		} else if (states[i].timestamp == state.timestamp) {
			states[i] = state;
			return;
		}
	}
	states.insert(states.begin() + i + 1, state);
}

void RealmClient::UpdateEntityCurrentState(uint64_t localId, uint64_t serverId)
{
	ComponentMovementState state;
	ExecuteMovementUpdate(localId, &state);
	flecs::entity entity = Entity(localId);
	entity.set<ComponentMovementState>(state);
	entity.set<ComponentLastAuthoritativeMovementState>({state});
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
		LOG_ERROR("Character %lu does not have shape component", entityId);
		return;
	}
	ComponentLastAuthoritativeMovementState *lastAuthoritativeState =
		entity.try_get_mut<ComponentLastAuthoritativeMovementState>();
	if (lastAuthoritativeState == nullptr) {
		LOG_ERROR("Character %lu does not have shape component", entityId);
		return;
	}
	const ComponentMovementParameters *movementParams =
		entity.try_get<ComponentMovementParameters>();
	if (movementParams == nullptr) {
		LOG_ERROR("Character %lu does not have shape component", entityId);
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
			if (states[id].timestamp <= currentTick) {
				break;
			}
		}

		if (id >= 0) {
			if (lastAuthoritativeState->oldState != states[id]) {
				lastAuthoritativeState->oldState = states[id];
				*currentState = states[id];
			}

			if (id + 1 < states.size()) {
				ComponentMovementState prev;
				if (id >= 0) {
					prev = states[id];
				} else {
					prev = lastAuthoritativeState->oldState;
				}
				ComponentMovementState next = states[id + 1];

				{
					EntitySystems::UpdateMovement(this, entity, *shape,
												  *currentState, {prev},
												  *movementParams);
				}

				glm::vec3 A = prev.pos;
				glm::vec3 B = next.pos;
				glm::vec3 V = prev.vel;
				Tick iDt = next.timestamp - prev.timestamp;
				float fullDt = iDt.v * TICK_DURATION_SECONDS;

				glm::vec3 a = (B - A - V * fullDt) / (fullDt * fullDt) * 2.0f;
				Tick iT = currentTick - prev.timestamp;
				float dt = iT.v * TICK_DURATION_SECONDS;
				float t = (float)(iT.v) / (float)(iDt.v);

				currentState->vel = V + a * dt;
				glm::vec3 P = A + V * dt + a * dt * dt * 0.5f;
				currentState->pos = currentState->pos * (1 - t) + P * t;
				currentState->rot = prev.rot * (1 - t) + next.rot * t;
				currentState->onGround = next.onGround;
				currentState->timestamp = currentTick;
			} else {
				*currentState = states.back();
			}

			auto las = *lastAuthoritativeState;
			las.oldState = *currentState;

			EntitySystems::UpdateMovement(this, entity, *shape, *currentState,
										  las, *movementParams);
		} else {
			*state = states[0];
		}
		if (id > 10) {
			states.erase(states.begin(), states.begin() + id - 3);
		}
	} else {
		EntitySystems::UpdateMovement(this, entity, *shape, *currentState,
									  *lastAuthoritativeState, *movementParams);
	}
	*state = *currentState;
}

void RealmClient::RegisterObservers()
{
	RegisterObserver(
		flecs::OnAdd, +[](flecs::entity entity, const ComponentMovementState &,
						  const ComponentLastAuthoritativeMovementState &,
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
