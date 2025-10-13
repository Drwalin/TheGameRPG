#include "../../ICon7/include/icon7/Debug.hpp"

#include "../../ICon7/include/icon7/Time.hpp"

#include "../include/EntitySystems.hpp"
#include "../include/EntityEvent.hpp"
#include "../include/EntityComponents.hpp"
#include "../include/RegistryComponent.hpp"

#include "../include/Realm.hpp"

Realm::Realm()
	: statsOneEpochDuration("!!UNINITIALIZED!!"), ecs(ecs_mini()),
	  collisionWorld(this)
{
	Realm::RegisterObservers();
}

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
	ecs.each([](flecs::entity entity, TagAllEntity) { entity.destruct(); });
	ecs.defer_end();
	collisionWorld.Clear();
}

void Realm::Init(const std::string &realmName)
{
	statsOneEpochDuration.SetName("One_Epoch_Duration:" + realmName + ",[ms]");

	this->realmName = realmName;
	timer.Start(tickDuration);

	ecs.set<RealmPtr>(RealmPtr{this});
}

uint64_t Realm::NewEntity()
{
	flecs::entity entity = ecs.entity();
	entity.add<TagAllEntity>();
	return entity.id();
}

void Realm::RemoveEntity(uint64_t entity) { Entity(entity).destruct(); }

void Realm::RegisterObservers()
{
	collisionWorld.RegisterObservers(this);

	ecs.observer<ComponentLastAuthoritativeMovementState,
				 ComponentMovementParameters>()
		.event(flecs::OnSet)
		.each([this](flecs::entity entity,
					 const ComponentLastAuthoritativeMovementState &lastState,
					 const ComponentMovementParameters &movementParams) {
			const ComponentShape *shape = entity.try_get<ComponentShape>();
			if (shape == nullptr) {
				return;
			}
			if (entity.try_get<ComponentMovementState>() == nullptr) {
				return;
			}

			ComponentMovementState currentState = lastState.oldState;
			EntitySystems::UpdateMovement(this, entity, *shape, currentState,
										  lastState, movementParams);
			entity.set<ComponentMovementState>(currentState);
		});

	ecs.observer<ComponentEventsQueue, const ComponentMovementParameters>()
		.event(flecs::OnAdd)
		.each([this](flecs::entity entity, ComponentEventsQueue &eventsQueue,
					 const ComponentMovementParameters &) {
			static EntityEventTemplate defaultMovementEvent{
				"ExecuteMovementUpdate",
				+[](Realm *realm, Tick scheduledTick, Tick currentTick,
					uint64_t entityId) -> Tick {
					ComponentMovementState currentState;
					realm->ExecuteMovementUpdate(entityId, &currentState);

					return {1};
				}};
			EntityEvent event;
			event.dueTick = timer.currentTick + 1;
			event.event = &defaultMovementEvent;
			eventsQueue.ScheduleEvent(this, entity.id(), event);
		});
}

void Realm::RunOneEpoch()
{
	if (timer.nextTick > TickTimer::GetCurrentTimepoint()+icon7::time::milliseconds(5)) {
		return;
	}
	
	icon7::time::Point begin = icon7::time::GetTemporaryTimestamp();
	OneEpoch();
	icon7::time::Point end = icon7::time::GetTemporaryTimestamp();
	double duration = icon7::time::DeltaMSecBetweenTimepoints(begin, end);
	statsOneEpochDuration.PushValue(duration);
	statsOneEpochDuration.PrintAndResetStatsIfExpired(
		millisecondsBetweenStatsReport);
}

void Realm::OneEpoch()
{
	timer.Update(tickDuration);
	
	collisionWorld.StartEpoch();

	// TODO: update due queued entity events
	while (!eventsPriorityQueue.Empty()) {
		EntityEventEntry event = eventsPriorityQueue.Top();
		if (event.dueTick <= timer.currentTick) {
			eventsPriorityQueue.Pop();
			auto entity = Entity(event.entityId);
			if (entity.is_alive()) {
				((ComponentEventsQueue *)entity.try_get<ComponentEventsQueue>())
					->Update(timer.currentTick, event.entityId, this);
			}
		} else {
			break;
		}
	}
	
	// TODO: execute systems here

	collisionWorld.EndEpoch();
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

uint64_t Realm::CreateStaticEntity(const ComponentStaticTransform &transform,
								   const ComponentModelName &model,
								   const ComponentCollisionShape &shape)
{
	uint64_t entityId = NewEntity();
	flecs::entity entity = Entity(entityId);

	entity.set<ComponentStaticTransform>(transform);
	entity.set<ComponentModelName>(model);
	entity.set<ComponentCollisionShape>(shape);

	return entityId;
}

void Realm::ScheduleEntityEvent(flecs::entity entity, EntityEvent event)
{
	ComponentEventsQueue *eventsQueue = entity.try_get_mut<ComponentEventsQueue>();
	if (eventsQueue == nullptr) {
		LOG_FATAL("Trying to add event `%s` to entity without event queue.",
				  event.event->name);
		return;
	}

	eventsQueue->ScheduleEvent(this, entity.id(), event);
}
void Realm::ScheduleEntityEvent(uint64_t entityId, EntityEvent event)
{
	ScheduleEntityEvent(Entity(entityId), event);
}
