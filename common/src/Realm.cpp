#include <chrono>

#include <icon7/Debug.hpp>

#include "../include/EntitySystems.hpp"

#include "../include/Realm.hpp"

Realm::Realm() : collisionWorld(this)
{
	Realm::RegisterSystems();
	Realm::RegisterObservers();
}

Realm::~Realm()
{
	// TODO: how to use clear here?
	Realm::Clear();
}

void Realm::Clear()
{
	ecs.defer_begin();
	ecs.each([](flecs::entity entity) { entity.destruct(); });
	ecs.defer_end();

	collisionWorld.Clear();
}

void Realm::Init(const std::string &realmName)
{
	this->realmName = realmName;
	timer.Start();

	TerrainCollisionData col;
	col.vertices.push_back({-100, 0, -100});
	col.vertices.push_back({100, 0, -100});
	col.vertices.push_back({-100, 0, 100});
	col.vertices.push_back({100, 0, 100});
	col.indices.push_back(2);
	col.indices.push_back(1);
	col.indices.push_back(0);
	col.indices.push_back(2);
	col.indices.push_back(3);
	col.indices.push_back(1);
	collisionWorld.LoadStaticCollision(&col);
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

	auto s = *entity.get<EntityLastAuthoritativeMovementState>();
	s.oldState.timestamp = timer.currentTick;
	entity.set<EntityLastAuthoritativeMovementState>(s);
	entity.set<EntityMovementState>(s.oldState);

	return entity.id();
}

void Realm::RemoveEntity(uint64_t entity) { Entity(entity).destruct(); }

void Realm::RegisterObservers()
{
	collisionWorld.RegisterObservers(this);
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
	collisionWorld.RegisterSystems(this);
	systemsRunPeriodicallyByTimer.push_back(
		ecs.system<const EntityShape, EntityMovementState,
				   const EntityLastAuthoritativeMovementState,
				   const EntityMovementParameters>(
			   "EntityMovementPhysicsUpdateSystem")
			.each([this](flecs::entity entity, const EntityShape shape,
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
	int64_t deltaTicks = 0;
	timer.Update(maxDeltaTicks, &deltaTicks, nullptr);

	if (deltaTicks >= maxDeltaTicks) {
		for (auto &sys : systemsRunPeriodicallyByTimer) {
			auto beg = std::chrono::steady_clock::now();
			sys.run();
			auto end = std::chrono::steady_clock::now();
			auto dt = std::chrono::duration_cast<std::chrono::nanoseconds>(end-beg);
		}
		return true;
	} else {
		return false;
	}
}

void Realm::UpdateEntityAuthoritativeState(
	uint64_t entityId, const EntityLastAuthoritativeMovementState &state)
{
	flecs::entity entity = Entity(entityId);
	if (entity.is_alive()) {
		entity.set<EntityLastAuthoritativeMovementState>(state);
		entity.set<EntityMovementState>(state.oldState);
	}
}
