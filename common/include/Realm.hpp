#pragma once

#include <string>
#include <functional>

#include <flecs.h>

#include "TickTimer.hpp"
#include "CollisionWorld.hpp"
#include "EntityEvent.hpp"
#include "StatsCollector.hpp"

struct RealmPtr {
	class Realm *realm;
};

struct ComponentShape;
struct ComponentModelName;
struct ComponentMovementState;
struct ComponentLastAuthoritativeMovementState;
struct ComponentMovementParameters;
struct ComponentName;

class Realm
{
public:
	Realm();
	virtual ~Realm();

	virtual void Destroy();
	virtual void Clear();

	virtual void Init(const std::string &realmName);

	virtual uint64_t NewEntity();
	void RemoveEntity(uint64_t entity);

	void RegisterObservers();

	// returns false if was not busy or if does not need to be busy
	bool RunOneEpoch();

	virtual void UpdateEntityAuthoritativeState(
		uint64_t entityId,
		const ComponentLastAuthoritativeMovementState &state);

	virtual void ExecuteMovementUpdate(uint64_t entityId,
									   ComponentMovementState *stateOut) = 0;

	uint64_t CreateStaticEntity(const ComponentStaticTransform &transform,
								const ComponentModelName &model,
								const ComponentCollisionShape &shape);

	template <typename FF> void TrueDeferWhenNeeded(FF &&func);

public:
	void ScheduleEntityEvent(flecs::entity entity, EntityEvent event);
	void ScheduleEntityEvent(uint64_t entityId, EntityEvent event);

protected:
	// returns false if was not busy or if does not need to be busy
	virtual bool OneEpoch();
	StatsCollector statsOneEpochDuration;

	icon7::time::Diff millisecondsBetweenStatsReport = icon7::time::seconds(60);

public:
	TickTimer timer;
	int64_t minMovementDeltaTicks = 50;
	int64_t maxMovementDeltaTicks = 200;

	float gravity = -9.81f;

	flecs::world ecs;

	CollisionWorld collisionWorld;

	std::string realmName;

	// TODO: fill this
	EntityEventPriorityQueue eventsPriorityQueue;

public:
	template <typename... TArgs>
	auto _GetObserver(std::function<void(flecs::entity, TArgs...)> &func)
	{
		return ecs.observer<TArgs...>();
	}

	template <typename Fun>
	void RegisterObserver(flecs::entity_t event, Fun &&func)
	{
		decltype(std::function(*&func)) f;
		_GetObserver(f).event(event).each(std::move(func));
	}

public: // accessors
	template <typename T> void AssureComponent(uint64_t entity)
	{
		Entity(entity).add<T>();
	}

	template <typename T> void SetComponent(uint64_t entity, const T &value)
	{
		Entity(entity).set<T>(value);
	}
	template <typename T> bool HasComponent(uint64_t entity) const
	{
		return Entity(entity).has<T>();
	}
	template <typename T> const T *GetComponent(uint64_t entity) const
	{
		return Entity(entity).get<T>();
	}
	template <typename T> T *AccessComponent(uint64_t entity) const
	{
		return (T *)Entity(entity).get<T>();
	}

	template <typename T> void RemoveComponent(uint64_t entity)
	{
		Entity(entity).remove<T>();
	}

	inline flecs::entity Entity(uint64_t entity)
	{
		return flecs::entity(ecs.get_world(), entity);
	}
	inline const flecs::entity Entity(uint64_t entity) const
	{
		return flecs::entity(ecs.get_world(), entity);
	}
	inline bool IsEntityAlive(uint64_t entity) const
	{
		return flecs::entity(ecs.get_world(), entity).is_alive();
	}
};

template <typename FF> void Realm::TrueDeferWhenNeeded(FF &&func)
{
	if (ecs.is_deferred()) {
		struct CtxData {
			FF func;
		};
		void (*f)(ecs_world_t *, void *) = +[](ecs_world_t *world, void *_ctx) {
			CtxData *ctx = (CtxData *)_ctx;
			ctx->func();
			delete ctx;
		};
		CtxData *ctx = new CtxData{std::move(func)};
		ecs.run_post_frame(f, ctx);
	} else {
		func();
	}
}
