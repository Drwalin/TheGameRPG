#pragma once

#include <string>
#include <functional>

#include <glm/glm.hpp>
#include <flecs.h>

#include "Timer.hpp"
#include "CollisionWorld.hpp"
#include "EntityEvent.hpp"

struct RealmPtr
{
	class Realm *realm;
};

class Realm
{
public:
	Realm();
	virtual ~Realm();

	virtual void Destroy();
	virtual void Clear();

	virtual void Init(const std::string &realmName);

	uint64_t NewEntity();
	void RemoveEntity(uint64_t entity);

	void RegisterObservers();

	// returns false if was not busy or if does not need to be busy
	virtual bool OneEpoch();

	virtual void UpdateEntityAuthoritativeState(
		uint64_t entityId,
		const ComponentLastAuthoritativeMovementState &state);

	virtual ComponentMovementState ExecuteMovementUpdate(uint64_t entityId) = 0;

	uint64_t CreateStaticEntity(ComponentStaticTransform transform,
								ComponentModelName model,
								ComponentStaticCollisionShapeName shape);

	virtual bool GetCollisionShape(std::string collisionShapeName,
								   TerrainCollisionData *data) = 0;

public:
	Timer timer;
	int64_t minMovementDeltaTicks = 50;
	int64_t maxMovementDeltaTicks = 200;
	int64_t ticksBeforeIgnoringInputMovement = 500;

	float gravity = -9.81f;

	flecs::world ecs;

	CollisionWorld collisionWorld;

	std::string realmName;

	// TODO: fill this
	EntityEventPriorityQueue eventsPriorityQueue;

public:
	flecs::query<const ComponentShape, ComponentMovementState,
				 const ComponentLastAuthoritativeMovementState,
				 const ComponentMovementParameters>
		queryEntityForMovementUpdate;

public:
	template <typename... TArgs>
	auto GetObserver(std::function<void(flecs::entity, TArgs...)> &func)
	{
		return ecs.observer<TArgs...>();
	}

	template <typename Fun>
	void RegisterObserver(flecs::entity_t event, Fun &&func)
	{
		decltype(std::function(*&func)) f;
		GetObserver(f).event(event).each(std::move(func));
	}

public: // accessors
	template <typename T, typename... Args>
	void EmplaceComponent(uint64_t entity, Args &&...args)
	{
		Entity(entity).emplace<T>(args...);
	}
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
