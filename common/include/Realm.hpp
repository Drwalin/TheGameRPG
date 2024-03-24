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
	void Destroy();

	virtual void Init(const std::string &realmName);

	uint64_t NewEntity();
	void RemoveEntity(uint64_t entity);

	virtual void RegisterObservers();
	virtual void RegisterSystems();

	virtual void OneEpoch();

public:
	Timer timer;
	uint64_t minDeltaTicks = 30;
	uint64_t maxDeltaTicks = 200;
	uint64_t ticksBeforeIgnoringInputMovement = 500;
	
	float gravity = -9.81f;

	flecs::world ecs;
	std::vector<flecs::system> systemsRunPeriodicallyByTimer;
	
	CollisionWorld collisionWorld;

public: // accessors
	template <typename T, typename... Args>
	void EmplaceComponent(uint64_t entity, Args &&...args)
	{
		Entity(entity).emplace(args...);
	}
	template <typename T> void AddComponent(uint64_t entity, T &&value)
	{
		Entity(entity).add<T>(std::move(value));
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
