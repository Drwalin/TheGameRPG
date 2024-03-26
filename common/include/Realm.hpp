#pragma once

#include <cstdio>
#include <cinttypes>

#include <string>
#include <vector>

#include <flecs.h>
#include <glm/glm.hpp>

#include "Timer.hpp"
#include "CollisionWorld.hpp"

class Realm
{
public:
	Realm();
	virtual ~Realm();
	
	void Destroy();

	virtual void Init(const std::string &realmName);

	uint64_t NewEntity();
	void RemoveEntity(uint64_t entity);

	virtual void RegisterObservers();
	virtual void RegisterSystems();

	// returns false if was not busy
	virtual bool OneEpoch();
	
	virtual void UpdateEntityAuthoritativeState(uint64_t entityId, const EntityLastAuthoritativeMovementState &state);

public:
	Timer timer;
	uint64_t minDeltaTicks = 50;
	uint64_t maxDeltaTicks = 200;
	uint64_t ticksBeforeIgnoringInputMovement = 500;
	
	float gravity = -9.81f;

	flecs::world ecs;
	std::vector<flecs::system> systemsRunPeriodicallyByTimer;
	
	CollisionWorld collisionWorld;
	
	std::string realmName;
	
public:
	
	template<typename ...TArgs>
	auto GetObserver(std::function<void(flecs::entity, TArgs...)> &func)
	{
		return ecs.observer<TArgs...>();
	}
	
	template<typename Fun>
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
	
	template <typename T> void SetComponent(uint64_t entity, T &&value)
	{
		Entity(entity).set<T>(std::move(value));
	}
	template <typename T> bool HasComponent(uint64_t entity) const
	{
		return Entity(entity).has<T>();
	}
	template <typename T> T *GetComponent(uint64_t entity)
	{
		return Entity(entity).get<T>();
	}
	template <typename T> const T *GetComponent(uint64_t entity) const
	{
		return Entity(entity).get<T>();
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
};
