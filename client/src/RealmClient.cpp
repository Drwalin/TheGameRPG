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
	
	for (int i=states.size()-1; i>=0; --i) {
		if (states[i].timestamp < state.timestamp) {
			states.insert(states.begin()+i+1, state);
			return;
		} else if (states[i].timestamp == state.timestamp) {
			states[i] = state;
			return;
		}
	}
	
	states.insert(states.begin(), state);
	return;
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
	if (entityId != gameClient->localPlayerEntityId && _states != nullptr && _states->states.size() > 0) {
		auto &states = _states->states;
		
		int id = states.size()-1;
		
		for (; id>=0; --id) {
			if (states[id].timestamp <= timer.currentTick) {
				break;
			}
		}
		
		if (id >= 0) {
			if (lastAuthoritativeState->oldState != states[id]) {
				lastAuthoritativeState->oldState = states[id];
				*currentState = states[id];
			}
		}
		
		EntitySystems::UpdateMovement(
				this, entity, *shape, *currentState,
				*lastAuthoritativeState, *movementParams);
		
		if (id+1 < states.size()) {
			EntityMovementState prev;
			if (id >= 0) {
				prev = states[id];
			} else {
				prev = lastAuthoritativeState->oldState;
			}
			EntityMovementState next = states[id+1];
			
			glm::vec3 A = prev.pos;
			glm::vec3 B = next.pos;
			glm::vec3 V = prev.vel;
			int64_t iDt = next.timestamp - prev.timestamp;
			float fullDt = iDt * 0.001f;
			
			glm::vec3 a = (B - A - V*fullDt) / (fullDt * fullDt) * 2.0f;
			
			int64_t iT = timer.currentTick - prev.timestamp;
			float dt = iT * 0.001f;
			
			float t = (float)iT / (float)iDt;
			
			currentState->vel = V + a*dt;
			
			glm::vec3 P = A + V*dt + a*dt*dt*0.5f;
			
			currentState->pos =
				currentState->pos * (1-t) +
				P * t;
			
			currentState->rot =
				prev.rot * (1-t) +
				next.rot * t;
		}
		
		if (id > 10) {
			states.erase(states.begin(), states.begin()+id-3);
		}
	} else {
		EntitySystems::UpdateMovement(
				this, entity, *shape, *currentState,
				*lastAuthoritativeState, *movementParams);
	}
	
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
