#pragma once

#include <cstdio>
#include <cinttypes>

#include <string>

#include <glm/glm.hpp>

#include "CollisionWorld.hpp"
#include "EntityBase.hpp"
#include "Timer.hpp"

class RealmBase
{
public:
	RealmBase();
	virtual ~RealmBase();
	
	virtual void InitByRealmName(const std::string &realmName) = 0;
	
	uint64_t AddEntity();
	void DestroyEntity(uint64_t entityId);
	void SetEntityPosition(uint64_t entityId, glm::vec3 pos);
	void SetEntityModel(uint64_t entityId, const std::string &modelName,
					   float width, float height);
	
	virtual EntityBase *GetEntity(uint64_t entityId) = 0;
	
protected:
	virtual uint64_t _InternalAddEntity() = 0;
	virtual uint64_t _InternalDestroyEntity(uint64_t entityId) = 0;
	
public:
	std::string realmName;
	CollisionWorld *collisionWorld = nullptr;
	float gravity = 9.81;
	
	Timer timer;
	uint64_t minDeltaTicks = 30;
	uint64_t maxDeltaTicks = 100;
	
	uint64_t despawnAfterNoUpdatesForTicks = 10*1000;
	uint64_t ticksBeforeIgnoringInputMovement = 400;
};
