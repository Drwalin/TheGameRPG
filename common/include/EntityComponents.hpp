#pragma once

#include <random>
#include <string>

#include <glm/glm.hpp>
#include <glm/ext/quaternion_float.hpp>
#include <glm/ext/quaternion_common.hpp>

#include <icon7/Debug.hpp>

#include "GlmSerialization.hpp"

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wall"
static inline float R()
{
	static std::random_device rd;
	static std::mt19937_64 mt(rd());
	static std::uniform_real_distribution<float> dist(-50, 50);
	return dist(mt);
}
#pragma clang diagnostic pop

#define MV(VAR) this->VAR = o.VAR;

#define DEFAULT_CONSTRUCTORS_AND_MOVE(STRUCT, CODE)                            \
	inline STRUCT(const STRUCT &o) { CODE }                                    \
	inline STRUCT(STRUCT &o) { CODE }                                          \
	inline STRUCT(STRUCT &&o) { CODE }                                         \
	inline STRUCT() {}                                                         \
	inline ~STRUCT() {}                                                        \
	inline STRUCT &operator=(const STRUCT &o) { CODE return *this; }           \
	inline STRUCT &operator=(STRUCT &o) { CODE return *this; }                 \
	inline STRUCT &operator=(STRUCT &&o) { CODE return *this; }

struct ComponentShape {
	float height = 1.65f;
	float width = 0.5f;

	template <typename S> S &__ByteStream_op(S &s)
	{
		s.op(height);
		s.op(width);
		return s;
	}

	ComponentShape(float h, float w) : height(h), width(w) {}

	DEFAULT_CONSTRUCTORS_AND_MOVE(ComponentShape, {MV(height) MV(width)});
};

struct ComponentMovementState {
	int64_t timestamp = 0;
	glm::vec3 pos = {R(), 70, R()};
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

	template <typename S> S &__ByteStream_op(S &s)
	{
		s.op(timestamp);
		s.op(pos);
		s.op(vel);
		s.op(rot);
		s.op(onGround);
		return s;
	}

	DEFAULT_CONSTRUCTORS_AND_MOVE(ComponentMovementState,
								  {MV(timestamp) MV(pos) MV(vel) MV(rot)
									   MV(onGround)});
};

struct ComponentLastAuthoritativeMovementState {
	ComponentMovementState oldState;

	template <typename S> S &__ByteStream_op(S &s)
	{
		return oldState.__ByteStream_op(s);
	}

	DEFAULT_CONSTRUCTORS_AND_MOVE(ComponentLastAuthoritativeMovementState,
								  MV(oldState));
};

struct ComponentName {
	std::string name;

	template <typename S> S &__ByteStream_op(S &s)
	{
		s.op(name);
		return s;
	}

	ComponentName(std::string n) : name(n) {}

	DEFAULT_CONSTRUCTORS_AND_MOVE(ComponentName, MV(name));
};

struct ComponentMovementParameters {
	float maxMovementSpeedHorizontal = 5.0f;
	float stepHeight = 0.25f;

	template <typename S> S &__ByteStream_op(S &s)
	{
		s.op(maxMovementSpeedHorizontal);
		s.op(stepHeight);
		return s;
	}

	DEFAULT_CONSTRUCTORS_AND_MOVE(ComponentMovementParameters,
								  {MV(maxMovementSpeedHorizontal)
									   MV(stepHeight)});
};

struct ComponentModelName {
	std::string modelName;

	template <typename S> S &__ByteStream_op(S &s)
	{
		s.op(modelName);
		return s;
	}

	inline ComponentModelName(std::string modelName)
	{
		this->modelName = modelName;
	}

	DEFAULT_CONSTRUCTORS_AND_MOVE(ComponentModelName, MV(modelName));
};

struct ComponentStaticTransform {
	glm::vec3 pos = {0, 0, 0};
	glm::quat rot = {0, 0, 0, 1};
	glm::vec3 scale = {1, 1, 1};

	template <typename S> S &__ByteStream_op(S &s)
	{
		s.op(pos);
		s.op(rot);
		s.op(scale);
		return s;
	}

	DEFAULT_CONSTRUCTORS_AND_MOVE(ComponentStaticTransform,
								  {MV(pos) MV(rot) MV(scale)});
};

struct ComponentStaticCollisionShapeName {
	std::string shapeName;

	template <typename S> S &__ByteStream_op(S &s)
	{
		s.op(shapeName);
		return s;
	}

	inline ComponentStaticCollisionShapeName(std::string shapeName)
	{
		this->shapeName = shapeName;
	}

	DEFAULT_CONSTRUCTORS_AND_MOVE(ComponentStaticCollisionShapeName,
								  MV(shapeName));
};

struct ComponentCharacterSheet {
	float useRange = 4.0f;

	template <typename S> S &__ByteStream_op(S &s)
	{
		s.op(useRange);
		return s;
	}

	DEFAULT_CONSTRUCTORS_AND_MOVE(ComponentCharacterSheet, MV(useRange));
};
