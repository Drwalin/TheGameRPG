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
	ecs.each([](flecs::entity entity, ComponentShape &) { entity.destruct(); });
	ecs.defer_end();
	ecs.defer_begin();
	ecs.each([](flecs::entity entity, ComponentEventsQueue &) {
		entity.destruct();
	});
	ecs.defer_end();
	ecs.defer_begin();
	ecs.each([](flecs::entity entity, ComponentMovementState &) {
		entity.destruct();
	});
	ecs.defer_end();
	ecs.defer_begin();
	ecs.each([](flecs::entity entity, ComponentName &) { entity.destruct(); });
	ecs.defer_end();
	ecs.defer_begin();
	ecs.each([](flecs::entity entity, ComponentStaticCollisionShapeName &) {
		entity.destruct();
	});
	ecs.defer_end();
	ecs.defer_begin();
	ecs.each([](flecs::entity entity, ComponentStaticTransform &) {
		entity.destruct();
	});
	ecs.defer_end();
	ecs.defer_begin();
	ecs.each(
		[](flecs::entity entity, ComponentModelName &) { entity.destruct(); });
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

	ecs.observer<ComponentLastAuthoritativeMovementState>()
		.event(flecs::OnSet)
		.each([this](flecs::entity entity,
					 const ComponentLastAuthoritativeMovementState &lastState) {
			const ComponentShape *shape = ecs.get<ComponentShape>();
			if (shape == nullptr) {
				return;
			}

			ComponentMovementState currentState = lastState.oldState;
			auto movementParams = entity.get<ComponentMovementParameters>();
			EntitySystems::UpdateMovement(this, entity, *shape, currentState,
										  lastState, *movementParams);
			entity.set<ComponentMovementState>(currentState);
		});

	ecs.observer<ComponentEventsQueue, const ComponentMovementParameters>()
		.event(flecs::OnAdd)
		.each([this](flecs::entity entity, ComponentEventsQueue &eventsQueue,
					 const ComponentMovementParameters &) {
			static EntityEventTemplate defaultMovementEvent{
				[](Realm *realm, int64_t scheduledTick, int64_t currentTick,
				   uint64_t entityId) {
					ComponentMovementState currentState =
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

					ComponentEventsQueue *eventsQueue =
						realm->AccessComponent<ComponentEventsQueue>(entityId);
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
		ecs.query<const ComponentShape, ComponentMovementState,
				  const ComponentLastAuthoritativeMovementState,
				  const ComponentMovementParameters>();
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
				((ComponentEventsQueue *)entity.get<ComponentEventsQueue>())
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
	uint64_t entityId, const ComponentLastAuthoritativeMovementState &state)
{
	flecs::entity entity = Entity(entityId);
	if (entity.is_alive()) {
		entity.set<ComponentLastAuthoritativeMovementState>(state);
		entity.set<ComponentMovementState>(state.oldState);
	}
}

uint64_t Realm::CreateStaticEntity(ComponentStaticTransform transform,
								   ComponentModelName model,
								   ComponentStaticCollisionShapeName shape)
{
	uint64_t entityId = NewEntity();
	flecs::entity entity = Entity(entityId);

	entity.set(transform);
	entity.set(model);
	entity.set(shape);

	return entityId;
}
