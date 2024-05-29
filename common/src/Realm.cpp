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
	entity.add<EntityEventsQueue>();

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
			if (shape == nullptr) {
				return;
			}
			/*
			int64_t dt = timer.currentTick - lastState.oldState.timestamp;
			if (dt >= minMovementDeltaTicks) {
			*/
			EntityMovementState currentState = lastState.oldState;
			auto movementParams = entity.get<EntityMovementParameters>();
			EntitySystems::UpdateMovement(this, entity, *shape, currentState,
										  lastState, *movementParams);
			entity.set<EntityMovementState>(currentState);
			/*
			} else {
				entity.set<$EntityMovementState>(lastState.oldState);
				collisionWorld.UpdateEntityBvh(entity.id(), *shape,
											   lastState.oldState.pos);
			}
			*/
		});

	ecs.observer<EntityEventsQueue>()
		.event(flecs::OnAdd)
		.each([this](flecs::entity entity, EntityEventsQueue &eventsQueue) {
			EntityEvent event;
			event.dueTick = timer.currentTick + 100;
			static EntityEventTemplate defaultMovementEvent{
				[](Realm *realm, int64_t scheduledTick, int64_t currentTick,
				   uint64_t entityId) {
					auto entity = realm->Entity(entityId);

					const EntityShape *shape = entity.get<EntityShape>();
					if (shape == nullptr) {
						return;
					}
					EntityMovementState *currentState =
						(EntityMovementState *)
							entity.get<EntityMovementState>();
					if (currentState == nullptr) {
						return;
					}
					const EntityLastAuthoritativeMovementState
						*lastAuthoritativeState =
							entity.get<EntityLastAuthoritativeMovementState>();
					if (lastAuthoritativeState == nullptr) {
						return;
					}
					const EntityMovementParameters *movementParams =
						entity.get<EntityMovementParameters>();
					if (movementParams == nullptr) {
						return;
					}
					EntityEventsQueue *eventsQueue = 
						(EntityEventsQueue *)entity.get<EntityEventsQueue>();
					if (eventsQueue == nullptr) {
						return;
					}
					
					LOG_DEBUG("Update movement");
					
					EntitySystems::UpdateMovement(
						realm, entity, *shape, *currentState,
						*lastAuthoritativeState, *movementParams);
					
					glm::vec3 v = currentState->vel;
					int64_t dt = realm->maxMovementDeltaTicks;;
					if (currentState->onGround == false) {
						dt = realm->minMovementDeltaTicks;;
					} else if (fabs(v.x) + fabs(v.y) + fabs(v.z) > 0.001) {
						dt = realm->minMovementDeltaTicks;;
					}
					EntityEvent event;
					event.dueTick = realm->timer.currentTick + dt;
					event.event = &defaultMovementEvent;
					eventsQueue->ScheduleEvent(realm, entity.id(), event);
				}};
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

		return true;
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
