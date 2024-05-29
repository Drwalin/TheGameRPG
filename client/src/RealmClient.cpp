#include "../../common/include/CollisionLoader.hpp"

#include "../include/GameClient.hpp"
#include "../include/EntityDataClient.hpp"
#include "../../common/include/EntitySystems.hpp"

#include "../include/RealmClient.hpp"

RealmClient::RealmClient(GameClient *gameClient) : gameClient(gameClient)
{
	this->minMovementDeltaTicks = 3;
	this->maxMovementDeltaTicks = 9;
	RealmClient::RegisterObservers();
}

RealmClient::~RealmClient() { peer = nullptr; }

void RealmClient::Init(const std::string &realmName)
{
	// TODO: load static realm data from database/disk
	Realm::Init(realmName);
	
	CollisionLoader loader;
	loader.LoadOBJ(std::string("assets/models/") + realmName + ".obj");
	collisionWorld.LoadStaticCollision(&loader.collisionData);
}

void RealmClient::Clear()
{
	Realm::Clear();
	peer = nullptr;
}

void RealmClient::Reinit(const std::string &realmName)
{
	Clear();
	Init(realmName);
}

bool RealmClient::OneEpoch()
{
	return Realm::OneEpoch();
}

void RealmClient::AddNewAuthoritativeMovementState(
	uint64_t localId, uint64_t serverId,
	EntityLastAuthoritativeMovementState _state)
{
	_state.oldState.timestamp += STATE_UPDATE_DELAY;
	EntityMovementState state = _state.oldState;
	EntityMovementHistory *movement = AccessComponent<EntityMovementHistory>(localId);
	auto &states = movement->states;
	int id = states.size()-1;
	if (states.size() == 0) {
		states.push_back(state);
		return;
	}
	if (states.size() == 1) {
		if (states[0].timestamp > state.timestamp) {
			states.insert(states.begin(), state);
			return;
		} else if (states[0].timestamp < state.timestamp) {
			states.push_back(state);
			return;
		} else {
			states[0] = state;
			return;
		}
	}
	for (; id >= 0; --id) {
		if (states[id].timestamp <= state.timestamp) {
			break;
		}
	}
	++id;
	if (id == states.size()) {
		states.push_back(state);
		return;
	}
	if (states[id].timestamp == state.timestamp) {
		states[id] = state;
	} else {
		states.insert(states.begin() + id, state);
	}
}

void RealmClient::UpdateEntityCurrentState(uint64_t localId, uint64_t serverId)
{
	ExecuteMovementUpdate(localId);
}

EntityMovementState RealmClient::ExecuteMovementUpdate(uint64_t entityId)
{
	auto entity = Entity(entityId);

	const EntityShape *shape = entity.get<EntityShape>();
	if (shape == nullptr) {
		return {};
	}
	EntityMovementState *currentState =
		(EntityMovementState *)
		entity.get<EntityMovementState>();
	if (currentState == nullptr) {
		return {};
	}
	EntityLastAuthoritativeMovementState *lastAuthoritativeState =
		(EntityLastAuthoritativeMovementState *)entity.get<EntityLastAuthoritativeMovementState>();
	if (lastAuthoritativeState == nullptr) {
		return {};
	}
	const EntityMovementParameters *movementParams =
		entity.get<EntityMovementParameters>();
	if (movementParams == nullptr) {
		return {};
	}
	
	EntityMovementHistory *_states = (EntityMovementHistory *)entity.get<EntityMovementHistory>();
	if (_states != nullptr) {
		auto &states = _states->states;
		
		int id = states.size()-1;
		for (; id >= 0; --id) {
			if (states[id].timestamp <= timer.currentTick) {
				break;
			}
		}
		++id;
		if (id < states.size()) {
			lastAuthoritativeState->oldState = states[id];
			*currentState = states[id];
		}
	}

	EntitySystems::UpdateMovement(
			this, entity, *shape, *currentState,
			*lastAuthoritativeState, *movementParams);
	
	EntityLastAuthoritativeMovementState state;
	state.oldState = *currentState;
	glm::vec3 p1 = state.oldState.pos, p2 = state.oldState.vel;
	LOG_DEBUG(
			"recv  other [>%lu]: pos (%f, %f, %f), vel (%f, %f, %f),    %s",
			entityId, p1.x, p1.y, p1.z, p2.x, p2.y, p2.z,
			state.oldState.onGround ? "on ground" : "falling");
	
	return *currentState;
}

void RealmClient::RegisterObservers()
{
	RegisterObserver(flecs::OnAdd,
					 [this](flecs::entity entity, const EntityMovementState &,
							const EntityLastAuthoritativeMovementState &,
							const EntityName, const EntityModelName,
							const EntityShape, const EntityName &name) {
						 gameClient->OnEntityAdd(entity.id());
						 entity.add<EntityMovementHistory>();
					 });

	RegisterObserver(flecs::OnRemove,
					 [this](flecs::entity entity, const EntityMovementState &) {
						 gameClient->OnEntityRemove(entity.id());
					 });

	RegisterObserver(flecs::OnSet, [this](flecs::entity entity,
										  const EntityModelName &model) {
		gameClient->OnEntityModel(entity.id(), model);
	});

	RegisterObserver(flecs::OnSet,
					 [this](flecs::entity entity, const EntityShape &shape) {
						 gameClient->OnEntityShape(entity.id(), shape);
					 });
}
