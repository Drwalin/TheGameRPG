#pragma once

#include <string>

#include "../../thirdparty/glm/glm/ext/vector_float3.hpp"
#include "../../thirdparty/glm/glm/ext/quaternion_float.hpp"

#include "ComponentsUtility.hpp"

struct ComponentShape {
	float height = 1.65f;
	float width = 0.5f;

	ComponentShape(float h, float w) : height(h), width(w) {}

	BITSCPP_BYTESTREAM_OP_DECLARATIONS();
	DEFAULT_CONSTRUCTORS_AND_MOVE(ComponentShape, {MV(height) MV(width)});
};

struct ComponentMovementState {
	int64_t timestamp = 0;
	glm::vec3 pos = {0, 0, 0};
	glm::vec3 vel = {0, 0, 0};
	glm::vec3 rot = {0, 0, 0};
	bool onGround = false;

	inline bool operator==(const ComponentMovementState &o) const
	{
		return timestamp == o.timestamp && pos == o.pos && vel == o.vel &&
			   rot == o.rot && onGround == o.onGround;
	}

	inline bool operator!=(const ComponentMovementState &o) const
	{
		return timestamp != o.timestamp || pos != o.pos || vel != o.vel ||
			   rot != o.rot || onGround != o.onGround;
	}

	BITSCPP_BYTESTREAM_OP_DECLARATIONS();
	DEFAULT_CONSTRUCTORS_AND_MOVE(ComponentMovementState,
								  {MV(timestamp) MV(pos) MV(vel) MV(rot)
									   MV(onGround)});
};

struct ComponentLastAuthoritativeMovementState {
	ComponentMovementState oldState;

	ComponentLastAuthoritativeMovementState(ComponentMovementState state)
		: oldState(state)
	{
	}

	BITSCPP_BYTESTREAM_OP_DECLARATIONS();
	DEFAULT_CONSTRUCTORS_AND_MOVE(ComponentLastAuthoritativeMovementState,
								  MV(oldState));
};

struct ComponentName {
	std::string name;

	ComponentName(std::string name) : name(name) {}

	BITSCPP_BYTESTREAM_OP_DECLARATIONS();
	DEFAULT_CONSTRUCTORS_AND_MOVE(ComponentName, MV(name));
};

struct ComponentMovementParameters {
	float maxMovementSpeedHorizontal = 5.0f;
	float stepHeight = 0.25f;

	BITSCPP_BYTESTREAM_OP_DECLARATIONS();
	DEFAULT_CONSTRUCTORS_AND_MOVE(ComponentMovementParameters,
								  {MV(maxMovementSpeedHorizontal)
									   MV(stepHeight)});
};

struct ComponentModelName {
	std::string modelName;

	ComponentModelName(std::string modelName) : modelName(modelName) {}

	bool operator==(const ComponentModelName &o) const
	{
		return modelName == o.modelName;
	}

	BITSCPP_BYTESTREAM_OP_DECLARATIONS();
	DEFAULT_CONSTRUCTORS_AND_MOVE(ComponentModelName, MV(modelName));
};

struct ComponentStaticTransform {
	glm::vec3 pos = {0, 0, 0};
	glm::quat rot = {0, 0, 0, 1};
	glm::vec3 scale = {1, 1, 1};

	ComponentStaticTransform(glm::vec3 pos, glm::quat rot, glm::vec3 scale)
		: pos(pos), rot(rot), scale(scale)
	{
	}

	BITSCPP_BYTESTREAM_OP_DECLARATIONS();
	DEFAULT_CONSTRUCTORS_AND_MOVE(ComponentStaticTransform,
								  {MV(pos) MV(rot) MV(scale)});
};

struct ComponentStaticCollisionShapeName {
	std::string shapeName;

	ComponentStaticCollisionShapeName(std::string shapeName)
		: shapeName(shapeName)
	{
	}

	BITSCPP_BYTESTREAM_OP_DECLARATIONS();
	DEFAULT_CONSTRUCTORS_AND_MOVE(ComponentStaticCollisionShapeName,
								  MV(shapeName));
};
