#pragma once

#include <cstdio>
#include <cinttypes>

#include <string>

#include <glm/glm.hpp>

#include "GlmSerialization.hpp"

struct EntityShape {
	float height = 1.75f;
	float width = 0.5f;

	template <typename S> S &__ByteStream_op(S &s)
	{
		s.op(height);
		s.op(width);
		return s;
	}
};

struct EntityMovementState {
	uint64_t timestamp = 0;
	glm::vec3 pos = {10, 100, 10};
	glm::vec3 vel = {0, 0, 0};
	glm::vec3 rot = {0, 0, 0};
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

struct EntityLastAuthoritativeMovementState {
	EntityMovementState oldState;

	template <typename S> S &__ByteStream_op(S &s)
	{
		return oldState.__ByteStream_op(s);
	}
};

struct EntityName {
	std::string name;

	template <typename S> S &__ByteStream_op(S &s)
	{
		s.op(name);
		return s;
	}
};

struct EntityMovementParameters {
	float maxMovementSpeedHorizontal = 5.0f;
	float stepHeight = 0.25f;

	template <typename S> S &__ByteStream_op(S &s)
	{
		s.op(maxMovementSpeedHorizontal);
		s.op(stepHeight);
		return s;
	}
};

struct EntityModelName {
	std::string modelName;

	template <typename S> S &__ByteStream_op(S &s)
	{
		s.op(modelName);
		return s;
	}
};
