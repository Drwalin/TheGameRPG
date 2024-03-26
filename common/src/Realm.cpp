#include "../include/EntitySystems.hpp"

#include "../include/Realm.hpp"

Realm::Realm() : collisionWorld(this) {}

void Realm::Destroy()
{
	ecs.defer_begin();
	ecs.each([](flecs::entity entity) { entity.destruct(); });
	ecs.defer_end();
}

void Realm::Init(const std::string &realmName)
{
	this->realmName = realmName;
	RegisterSystems();
	RegisterObservers();
}

uint64_t Realm::NewEntity()
{
	flecs::entity entity = ecs.entity();
	// TODO: check what to add depending on some arguments: string, enum,
	// something else

	entity.add<EntityShape>();
	entity.add<EntityMovementState>();
	entity.add<EntityLastAuthoritativeMovementState>();
	entity.add<EntityName>();
	entity.add<EntityMovementParameters>();
	entity.add<EntityModelName>();

	return entity.id();
}

void Realm::RemoveEntity(uint64_t entity) { Entity(entity).destruct(); }

void Realm::RegisterObservers()
{
	ecs.observer<EntityLastAuthoritativeMovementState>()
		.event(flecs::OnSet)
		.each([this](flecs::entity entity,
					 const EntityLastAuthoritativeMovementState &lastState) {
			const EntityShape *shape = ecs.get<EntityShape>();
			if (shape == nullptr)
				return;
			int64_t dt = timer.currentTick - lastState.oldState.timestamp;
			if (dt >= minDeltaTicks) {
				EntityMovementState currentState = lastState.oldState;
				auto movementParams = entity.get<EntityMovementParameters>();
				EntitySystems::UpdateMovement(this, entity, *shape,
											  currentState, lastState,
											  *movementParams);
				entity.set<EntityMovementState>(currentState);
			} else {
				entity.set<EntityMovementState>(lastState.oldState);
				collisionWorld.UpdateEntityBvh(entity.id(), *shape,
											   lastState.oldState.pos);
			}
		});
}

void Realm::RegisterSystems()
{
	systemsRunPeriodicallyByTimer.push_back(
		ecs.system<const EntityShape, EntityMovementState,
				   const EntityLastAuthoritativeMovementState,
				   const EntityMovementParameters>(
			   "EntityMovementPhysicsUpdateSystem")
			.iter([this](flecs::entity entity, const EntityShape shape,
						 EntityMovementState &currentState,
						 const EntityLastAuthoritativeMovementState
							 &lastAuthoritativeState,
						 const EntityMovementParameters &movementParams) {
				EntitySystems::UpdateMovement(this, entity, shape, currentState,
											  lastAuthoritativeState,
											  movementParams);
			}));
}

bool Realm::OneEpoch()
{
	uint64_t deltaTicks = 0;
	timer.Update(maxDeltaTicks, &deltaTicks, nullptr);

	if (deltaTicks >= maxDeltaTicks) {
		for (auto &sys : systemsRunPeriodicallyByTimer) {
			sys.run();
		}
		return true;
	} else {
		return false;
	}
}
