#include <icon7/Debug.hpp>

#include "../include/EntitySystems.hpp"
#include "../include/EntityEvent.hpp"
#include "../include/EntityComponents.hpp"
#include "../include/RegistryComponent.hpp"
#include "icon7/Time.hpp"

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
	timer.Start();

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
			const ComponentShape *shape = entity.get<ComponentShape>();
			if (shape == nullptr) {
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
				+[](Realm *realm, int64_t scheduledTick, int64_t currentTick,
					uint64_t entityId) -> int64_t {
					ComponentMovementState currentState;
					realm->ExecuteMovementUpdate(entityId, &currentState);

					glm::vec3 v = currentState.vel;
					int64_t dt = realm->maxMovementDeltaTicks;

					if (currentState.onGround == false) {
						dt = realm->minMovementDeltaTicks;
					} else if (fabs(v.x) + fabs(v.y) + fabs(v.z) > 0.001) {
						dt = realm->minMovementDeltaTicks;
					}

					return dt;
				}};
			EntityEvent event;
			event.dueTick = timer.currentTick + 100;
			event.event = &defaultMovementEvent;
			eventsQueue.ScheduleEvent(this, entity.id(), event);
		});
}

bool Realm::RunOneEpoch()
{
	uint64_t begin = icon7::time::GetTimestamp();
	bool ret = OneEpoch();
	uint64_t end = icon7::time::GetTimestamp();
	double duration = icon7::time::DeltaMSecBetweenTimestamps(begin, end);
	statsOneEpochDuration.PushValue(duration);
	statsOneEpochDuration.PrintAndResetStatsIfExpired(
		millisecondsBetweenStatsReport);
	return ret;
}

bool Realm::OneEpoch()
{
	collisionWorld.StartEpoch();

	timer.Update();

	// TODO: update due queued entity events
	int executedEvents = 0;
	while (!eventsPriorityQueue.Empty()) {
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
		collisionWorld.EndEpoch();
		return false;
	} else {
		timer.Update();
		if (!eventsPriorityQueue.Empty()) {
			const EntityEventEntry &event = eventsPriorityQueue.Top();
			if (event.dueTick > timer.currentTick + 10) {
				collisionWorld.EndEpoch();
				return false;
			}
		} else {
			collisionWorld.EndEpoch();
			return false;
		}
	}
	collisionWorld.EndEpoch();
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

uint64_t
Realm::CreateStaticEntity(const ComponentStaticTransform &transform,
						  const ComponentModelName &model,
						  const ComponentStaticCollisionShapeName &shape)
{
	uint64_t entityId = NewEntity();
	flecs::entity entity = Entity(entityId);

	entity.set<ComponentStaticTransform>(transform);
	entity.set<ComponentModelName>(model);
	entity.set<ComponentStaticCollisionShapeName>(shape);

	return entityId;
}

void Realm::ScheduleEntityEvent(flecs::entity entity, EntityEvent event)
{
	ComponentEventsQueue *eventsQueue = entity.get_mut<ComponentEventsQueue>();
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
