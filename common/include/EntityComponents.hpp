#pragma once

#include <random>
#include <string>

#include <glm/glm.hpp>
#include <glm/ext/quaternion_float.hpp>
#include <glm/ext/quaternion_common.hpp>

#include "GlmSerialization.hpp"

static inline float R()
{
	static std::random_device rd;
	static std::mt19937_64 mt(rd());
	static std::uniform_real_distribution<float> dist(-50, 50);
	return dist(mt);
}

struct ComponentShape {
	float height = 1.75f;
	float width = 0.5f;

	template <typename S> S &__ByteStream_op(S &s)
	{
		s.op(height);
		s.op(width);
		return s;
	}
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
};

struct ComponentLastAuthoritativeMovementState {
	ComponentMovementState oldState;

	template <typename S> S &__ByteStream_op(S &s)
	{
		return oldState.__ByteStream_op(s);
	}
};

struct ComponentName {
	std::string name;

	template <typename S> S &__ByteStream_op(S &s)
	{
		s.op(name);
		return s;
	}
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
};

struct ComponentModelName {
	std::string modelName;

	template <typename S> S &__ByteStream_op(S &s)
	{
		s.op(modelName);
		return s;
	}
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
};

struct ComponentStaticCollisionShapeName {
	std::string shapeName;

	template <typename S> S &__ByteStream_op(S &s)
	{
		s.op(shapeName);
		return s;
	}
};
