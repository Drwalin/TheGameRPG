#include <algorithm>

#include "../../thirdparty/flecs/distr/flecs.h"

#include "../include/Realm.hpp"

#include "../include/EntityEvent.hpp"

void ComponentEventsQueue::InsertIntoRealmSchedule(Realm *realm,
												   uint64_t thisEntityId,
												   int64_t dueTick)
{
	EntityEventEntry entry;
	entry.dueTick = dueTick;
	entry.entityId = thisEntityId;
	realm->eventsPriorityQueue.Push(entry);
}

void ComponentEventsQueue::ScheduleEvent(Realm *realm, uint64_t thisEntityId,
										 EntityEvent event)
{
	if (!events.Empty()) {
		EntityEvent &top = events.Top();
		if (top.dueTick < event.dueTick) {
			event.scheduledInRealm = false;
			events.Push(event);
			return;
		} else if (top.dueTick == event.dueTick) {
			event.scheduledInRealm = true;
			events.Push(event);
			return;
		}
	}

	event.scheduledInRealm = true;
	events.Push(event);
	InsertIntoRealmSchedule(realm, thisEntityId, event.dueTick);
}

void ComponentEventsQueue::Update(int64_t currentTick, uint64_t entityId,
								  class Realm *realm)
{
	if (events.Empty()) {
		return;
	}

	{
		EntityEvent &event = events.Top();
		if (event.dueTick > currentTick) {
			InsertIntoRealmSchedule(realm, entityId, event.dueTick);
			event.scheduledInRealm = true;
			return;
		}
	}

	for (int i = 0; i < 128; ++i) {
		if (events.Empty()) {
			break;
		}
		EntityEvent event = events.Top();
		if (event.dueTick <= currentTick) {
			events.Pop();
			int64_t dt = event.event->callback(realm, event.dueTick,
											   currentTick, entityId);
			if (dt > 0) {
				event.dueTick = currentTick + dt;
				ScheduleEvent(realm, entityId, event);
			} else if (event.event->singleUse) {
				delete event.event;
			}
		} else {
			break;
		}
	}

	{
		EntityEvent &event = events.Top();
		if (event.scheduledInRealm == false) {
			InsertIntoRealmSchedule(realm, entityId, event.dueTick);
			event.scheduledInRealm = true;
			return;
		}
	}
}

int RegisterEntityEventQueueComponent(flecs::world &ecs)
{
	ecs.component<ComponentEventsQueue>();
	return 0;
}

void ComponentEventsQueue::EntityEventPriorityQueue::Push(const EntityEvent &v)
{
	storage.push_back(v);
	std::push_heap(storage.begin(), storage.end(), EntityEvent::Comparator());
}
void ComponentEventsQueue::EntityEventPriorityQueue::Pop()
{
	std::pop_heap(storage.begin(), storage.end(), EntityEvent::Comparator());
	storage.resize(storage.size() - 1);
}

void EntityEventPriorityQueue::Push(const EntityEventEntry &v)
{
	storage.push_back(v);
	std::push_heap(storage.begin(), storage.end(),
				   EntityEventEntry::Comparator());
}
void EntityEventPriorityQueue::Pop()
{
	std::pop_heap(storage.begin(), storage.end(),
				  EntityEventEntry::Comparator());
	storage.resize(storage.size() - 1);
}
