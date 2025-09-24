#pragma once

#include <string>

#include "../../thirdparty/Collision3D/SpatialPartitioning/glm/glm/ext/vector_float3.hpp"

#include "../../thirdparty/Collision3D/include/collision3d/CollisionShapes.hpp"

#include "CollisionFilters.hpp"
#include "ComponentsUtility.hpp"

struct ComponentShape {
	float height = 1.65f;
	float width = 0.5f;

	ComponentShape(float h, float w) : height(h), width(w) {}

	BITSCPP_BYTESTREAM_OP_DECLARATIONS()
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

	BITSCPP_BYTESTREAM_OP_DECLARATIONS()
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

	BITSCPP_BYTESTREAM_OP_DECLARATIONS()
	DEFAULT_CONSTRUCTORS_AND_MOVE(ComponentLastAuthoritativeMovementState,
								  MV(oldState));
};

struct ComponentName {
	std::string name;

	ComponentName(std::string name) : name(name) {}

	BITSCPP_BYTESTREAM_OP_DECLARATIONS()
	DEFAULT_CONSTRUCTORS_AND_MOVE(ComponentName, MV(name));
};

struct ComponentMovementParameters {
	float maxMovementSpeedHorizontal = 5.0f;
	float stepHeight = 0.25f;

	BITSCPP_BYTESTREAM_OP_DECLARATIONS()
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

	BITSCPP_BYTESTREAM_OP_DECLARATIONS()
	DEFAULT_CONSTRUCTORS_AND_MOVE(ComponentModelName, MV(modelName));
};

struct ComponentStaticTransform {
	Collision3D::Transform trans;
	float scale = 1.0f;

	ComponentStaticTransform(Collision3D::Transform trans, float scale)
		: trans(trans), scale(scale)
	{
	}

	BITSCPP_BYTESTREAM_OP_DECLARATIONS()
	DEFAULT_CONSTRUCTORS_AND_MOVE(ComponentStaticTransform,
								  {MV(trans) MV(scale)});
};

struct ComponentCollisionShape {
	Collision3D::AnyShape shape;
	uint32_t mask = FILTER_STATIC_OBJECT;

	ComponentCollisionShape(Collision3D::AnyShape shape) : shape(shape) {}

	BITSCPP_BYTESTREAM_OP_DECLARATIONS();
	DEFAULT_CONSTRUCTORS_AND_MOVE(ComponentCollisionShape,
								  {MV(shape) MV(mask)});
};
