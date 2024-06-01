#include <icon7/Debug.hpp>

#include "../include/EntitySystems.hpp"
#include "../include/EntityEvent.hpp"

#include "../include/Realm.hpp"

Realm::Realm() : collisionWorld(this) { Realm::RegisterObservers(); }

Realm::~Realm()
{
	// TODO: how to use clear here?
	Realm::Clear();
}

void Realm::Destroy()
{
	Clear();
	ecs.~world();
	new (&ecs) flecs::world();
}

void Realm::Clear()
{
	ecs.defer_begin();
	ecs.each([](flecs::entity entity, EntityShape &) { entity.destruct(); });
	ecs.defer_end();
	ecs.defer_begin();
	ecs.each(
		[](flecs::entity entity, EntityEventsQueue &) { entity.destruct(); });
	ecs.defer_end();
	ecs.defer_begin();
	ecs.each(
		[](flecs::entity entity, EntityMovementState &) { entity.destruct(); });
	ecs.defer_end();
	ecs.defer_begin();
	ecs.each([](flecs::entity entity, EntityName &) { entity.destruct(); });
	ecs.defer_end();
	ecs.defer_begin();
	ecs.each([](flecs::entity entity, EntityStaticCollisionShapeName &) {
		entity.destruct();
	});
	ecs.defer_end();
	ecs.defer_begin();
	ecs.each([](flecs::entity entity, EntityStaticTransform &) {
		entity.destruct();
	});
	ecs.defer_end();
	ecs.defer_begin();
	ecs.each(
		[](flecs::entity entity, EntityModelName &) { entity.destruct(); });
	ecs.defer_end();

	collisionWorld.Clear();
}

void Realm::Init(const std::string &realmName)
{
	this->realmName = realmName;
	timer.Start();
}

uint64_t Realm::NewEntity()
{
	flecs::entity entity = ecs.entity();
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
			if (shape == nullptr) {
				return;
			}

			EntityMovementState currentState = lastState.oldState;
			auto movementParams = entity.get<EntityMovementParameters>();
			EntitySystems::UpdateMovement(this, entity, *shape, currentState,
										  lastState, *movementParams);
			entity.set<EntityMovementState>(currentState);
		});

	ecs.observer<EntityEventsQueue, const EntityMovementParameters>()
		.event(flecs::OnAdd)
		.each([this](flecs::entity entity, EntityEventsQueue &eventsQueue,
					 const EntityMovementParameters &) {
			static EntityEventTemplate defaultMovementEvent{
				[](Realm *realm, int64_t scheduledTick, int64_t currentTick,
				   uint64_t entityId) {
					EntityMovementState currentState =
						realm->ExecuteMovementUpdate(entityId);

					glm::vec3 v = currentState.vel;
					int64_t dt = realm->maxMovementDeltaTicks;

					if (currentState.onGround == false) {
						dt = realm->minMovementDeltaTicks;
						;
					} else if (fabs(v.x) + fabs(v.y) + fabs(v.z) > 0.001) {
						dt = realm->minMovementDeltaTicks;
						;
					}
					EntityEvent event;
					event.dueTick = realm->timer.currentTick + dt;
					event.event = &defaultMovementEvent;

					EntityEventsQueue *eventsQueue =
						realm->AccessComponent<EntityEventsQueue>(entityId);
					if (eventsQueue == nullptr) {
						return;
					}

					eventsQueue->ScheduleEvent(realm, entityId, event);
				}};
			EntityEvent event;
			event.dueTick = timer.currentTick + 100;
			event.event = &defaultMovementEvent;
			eventsQueue.ScheduleEvent(this, entity.id(), event);
		});

	queryEntityForMovementUpdate =
		ecs.query<const EntityShape, EntityMovementState,
				  const EntityLastAuthoritativeMovementState,
				  const EntityMovementParameters>();
}

bool Realm::OneEpoch()
{
	timer.Update();

	// TODO: update due queued entity events
	int executedEvents = 0;
	while (true) {
		if (eventsPriorityQueue.Empty()) {
			break;
		}
		EntityEventEntry event = eventsPriorityQueue.Top();
		if (event.dueTick <= timer.currentTick) {
			eventsPriorityQueue.Pop();
			auto entity = Entity(event.entityId);
			if (entity.is_alive()) {
				((EntityEventsQueue *)entity.get<EntityEventsQueue>())
					->Update(timer.currentTick, event.entityId, this);
				++executedEvents;
			}
		} else {
			break;
		}
	}

	if (executedEvents == 0) {
		return false;
	} else {
		timer.Update();
		if (!eventsPriorityQueue.Empty()) {
			const EntityEventEntry &event = eventsPriorityQueue.Top();
			if (event.dueTick > timer.currentTick + 10) {
				return false;
			}
		} else {
			return false;
		}
	}
	return true;
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

uint64_t Realm::CreateStaticEntity(EntityStaticTransform transform,
								   EntityModelName model,
								   EntityStaticCollisionShapeName shape)
{
	TerrainCollisionData col;
	bool r = GetCollisionShape(shape.shapeName, &col);
	if (r || shape.shapeName == "") {
		uint64_t entityId = NewEntity();
		flecs::entity entity = Entity(entityId);

		entity.set(transform);
		entity.set(model);
		entity.set(shape);

		if (r) {
			collisionWorld.LoadStaticCollision(&col, transform);
		}

		return entityId;
	}
	return 0;
}
