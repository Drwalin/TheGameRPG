#pragma once

#include <cstdio>
#include <cinttypes>

#include <string>
#include <vector>
#include <memory>
#include <unordered_map>

#include <glm/glm.hpp>

#include "GlmSerialization.hpp"
#include "EntityShape.hpp"

class RealmBase;

struct EntityMovementState
{
	uint64_t timestamp = 0;
	glm::vec3 pos = {10,100,10};
	glm::vec3 vel = {0,0,0};
	glm::vec3 rot = {0,0,0};
	bool onGround = false;
	
	template <typename S> S &__ByteStream_op(S &s)
	{
		s.op(timestamp);
		s.op(pos);
		s.op(vel);
		s.op(rot);
		s.op(onGround);
		return s;
	}
};

struct EntityLongState
{
	uint64_t entityId = 0;
	
	EntityShape shape;
	
	float maxMovementSpeedHorizontal = 5;
	float stepHeight = 0.25;
	
	std::string modelName;
	std::string userName;
	
	template <typename S> S &__ByteStream_op(S &s)
	{
		s.op(entityId);
		s.op(shape.height);
		s.op(shape.width);
		s.op(maxMovementSpeedHorizontal);
		s.op(stepHeight);
		s.op(modelName);
		s.op(userName);
		return s;
	}
};

class EntityBase
{
public:
	EntityBase();
	virtual ~EntityBase();
	
	virtual void Init(RealmBase *realm, uint64_t entityId);
	
	virtual void Update(uint64_t currentTick);
	
	void SolvePlayerInput(const EntityMovementState &state);
	
	virtual void SetModel(const std::string &modelName, float height, float width);
	
public:
	RealmBase *realm = nullptr;
	
	union {
		uint64_t entityId;
		EntityLongState longState;
	};
	
	EntityMovementState lastAuthoritativeState;
	EntityMovementState currentState;
	
};
