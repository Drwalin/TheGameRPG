#pragma once

#include <string>

#include <glm/glm.hpp>
#include <glm/ext/quaternion_float.hpp>
#include <glm/ext/quaternion_common.hpp>

#include <icon7/Debug.hpp>
#include <icon7/ByteBuffer.hpp>
#include <icon7/ByteReader.hpp>
#include <icon7/ByteWriter.hpp>

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

	BITSCPP_BYTESTREAM_OP_DECLARATIONS();
	DEFAULT_CONSTRUCTORS_AND_MOVE(ComponentLastAuthoritativeMovementState,
								  MV(oldState));
};

struct ComponentName {
	std::string name;

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

	BITSCPP_BYTESTREAM_OP_DECLARATIONS();
	DEFAULT_CONSTRUCTORS_AND_MOVE(ComponentModelName, MV(modelName));
};

struct ComponentStaticTransform {
	glm::vec3 pos = {0, 0, 0};
	glm::quat rot = {0, 0, 0, 1};
	glm::vec3 scale = {1, 1, 1};

	BITSCPP_BYTESTREAM_OP_DECLARATIONS();
	DEFAULT_CONSTRUCTORS_AND_MOVE(ComponentStaticTransform,
								  {MV(pos) MV(rot) MV(scale)});
};

struct ComponentStaticCollisionShapeName {
	std::string shapeName;

	BITSCPP_BYTESTREAM_OP_DECLARATIONS();
	DEFAULT_CONSTRUCTORS_AND_MOVE(ComponentStaticCollisionShapeName,
								  MV(shapeName));
};
