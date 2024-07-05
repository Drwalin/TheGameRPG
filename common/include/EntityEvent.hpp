#pragma once

#include <cstdint>

#include <functional>
#include <vector>

#include "ComponentsUtility.hpp"

struct EntityEventEntry {
	/*
	 * Next event cannot be scheduled for later than 10 minutes
	 */
	inline const static int64_t MAX_TICKS_TO_SCHEDULE_IN_REALM = 1000 * 60 * 10;

	int64_t dueTick;
	uint64_t entityId;

	struct Comparator {
		inline bool operator()(const EntityEventEntry &lhs,
							   const EntityEventEntry &rhs) const
		{
			return lhs.dueTick > rhs.dueTick;
		}
	};
};

/*
 * Objects of type EntityEventTemplate should be global and with life time the
 * same as application
 */
struct EntityEventTemplate {
	const std::function<void(class Realm *, int64_t scheduledTick,
							 int64_t currentTick, uint64_t entityId)>
		callback;
	const bool destroy = false;
};

struct EntityEvent {
	int64_t dueTick = 0;
	EntityEventTemplate *event = nullptr;
	bool scheduledInRealm = false;

	struct Comparator {
		inline bool operator()(const EntityEvent &lhs,
							   const EntityEvent &rhs) const
		{
			return lhs.dueTick > rhs.dueTick;
		}
	};
};

struct ComponentEventsQueue {
	struct EntityEventPriorityQueue {
	public:
		EntityEvent &Top() { return storage.front(); }
		void Push(const EntityEvent &v);
		void Pop();
		size_t Size() const { return storage.size(); }
		bool Empty() const { return storage.empty(); }
		std::vector<EntityEvent> storage;
	} events;

	void InsertIntoRealmSchedule(Realm *realm, uint64_t thisEntityId,
								 int64_t dueTick);
	void ScheduleEvent(Realm *realm, uint64_t thisEntityId, EntityEvent event);
	void Update(int64_t currentTick, uint64_t entityId, class Realm *realm);

	DEFAULT_CONSTRUCTORS_AND_MOVE(ComponentEventsQueue, MV(events));
};

struct EntityEventPriorityQueue {
public:
	EntityEventEntry &Top() { return storage.front(); }
	void Push(const EntityEventEntry &v);
	void Pop();
	size_t Size() const { return storage.size(); }
	bool Empty() const { return storage.empty(); }
	std::vector<EntityEventEntry> storage;
};
