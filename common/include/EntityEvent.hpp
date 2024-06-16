#pragma once

#include <cstdint>

#include <functional>
#include <vector>
#include <algorithm>

template <typename T, typename TComp> class PriorityQueue
{
public:
	T &Top() { return storage[0]; }

	void Push(const T &v)
	{
		storage.push_back(v);
		std::push_heap(storage.begin(), storage.end(), TComp());
	}

	void Pop()
	{
		std::pop_heap(storage.begin(), storage.end(), TComp());
		storage.resize(storage.size() - 1);
	}

	size_t Size() const { return storage.size(); }

	bool Empty() const { return storage.empty(); }

	std::vector<T> storage;
};

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
	PriorityQueue<EntityEvent, EntityEvent::Comparator> events;

	void InsertIntoRealmSchedule(Realm *realm, uint64_t thisEntityId,
								 int64_t dueTick);
	void ScheduleEvent(Realm *realm, uint64_t thisEntityId, EntityEvent event);
	void Update(int64_t currentTick, uint64_t entityId, class Realm *realm);
};

using EntityEventPriorityQueue =
	PriorityQueue<EntityEventEntry, EntityEventEntry::Comparator>;
