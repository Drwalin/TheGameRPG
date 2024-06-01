
#include "../include/Realm.hpp"

#include "../include/EntityEvent.hpp"

void EntityEventsQueue::InsertIntoRealmSchedule(Realm *realm,
												uint64_t thisEntityId,
												int64_t dueTick)
{
	EntityEventEntry entry;
	entry.dueTick = dueTick;
	entry.entityId = thisEntityId;
	realm->eventsPriorityQueue.Push(entry);
}

void EntityEventsQueue::ScheduleEvent(Realm *realm, uint64_t thisEntityId,
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

void EntityEventsQueue::Update(int64_t currentTick, uint64_t entityId,
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

	while (true) {
		if (events.Empty()) {
			break;
		}
		EntityEvent event = events.Top();
		if (event.dueTick <= currentTick) {
			events.Pop();
			event.event->callback(realm, event.dueTick, currentTick, entityId);
			if (event.event->destroy) {
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
