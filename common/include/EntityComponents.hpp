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

struct ComponentShape {
	float height = 1.65f;
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
// 		if constexpr (std::is_same_v<S, bitscpp::ByteReader<true>> == false) {
// 			LOG_INFO("Write (%p, %lu %lu) component model: %s", modelName.c_str(), modelName.capacity(), modelName.size(), modelName.c_str());
// 			s.op(modelName);
// 		} else if constexpr (std::is_same_v<S, bitscpp::ByteReader<true>> == true) {
// 			s.op(modelName);
// 			LOG_INFO("Read (%p, %lu %lu) component model: %s", modelName.c_str(), modelName.capacity(), modelName.size(), modelName.c_str());
// 		}
		s.op(modelName);
		return s;
	}
	
	inline ComponentModelName(std::string modelName)
	{
// 		LOG_INFO("\t\tctor_args \t\t%p", this);
		this->modelName = modelName;
	}

	inline ComponentModelName()
	{
// 		LOG_INFO("\t\tctor \t\t%p", this);
		modelName = "";
	}
	inline ~ComponentModelName() {
// 		LOG_INFO("\t\tdtor \t\t%p", this);
	}
	inline ComponentModelName(const ComponentModelName &o)
		: modelName(o.modelName)
	{
// 		LOG_INFO("\t\tctor_copy \t%p (%p)", this, &o);
	}
	inline ComponentModelName(ComponentModelName &o)
		: modelName(o.modelName)
	{
// 		LOG_INFO("\t\tctor2_copy \t%p (%p)", this, &o);
	}
	inline ComponentModelName(ComponentModelName &&o)
		: modelName(std::move(o.modelName))
	{
// 		LOG_INFO("\t\tctor_move \t%p (%p)", this, &o);
	}

	inline ComponentModelName &operator=(const ComponentModelName &o)
	{
// 		LOG_INFO("\t\t\tcopy \t\t%p (%p)", this, &o);
		modelName = o.modelName;
		return *this;
	}
	inline ComponentModelName &operator=(ComponentModelName &o)
	{
// 		LOG_INFO("\t\t\t\tcopy2 \t\t%p (%p)", this, &o);
		modelName = o.modelName;
		return *this;
	}
	inline ComponentModelName &operator=(ComponentModelName &&o)
	{
// 		LOG_INFO("\t\t\tmove \t\t%p (%p)", this, &o);
		modelName = o.modelName;
		return *this;
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
	
	inline ComponentStaticCollisionShapeName(std::string shapeName)
	{
// 		LOG_INFO("\t\tctor_args \t\t%p", this);
		this->shapeName = shapeName;
	}

	inline ComponentStaticCollisionShapeName()
	{
// 		LOG_INFO("\t\tctor \t\t%p", this);
		shapeName = "";
	}
	inline ~ComponentStaticCollisionShapeName() {
// 		LOG_INFO("\t\tdtor \t\t%p", this);
	}
	inline ComponentStaticCollisionShapeName(const ComponentStaticCollisionShapeName &o)
		: shapeName(o.shapeName)
	{
// 		LOG_INFO("\t\tctor_copy \t%p (%p)", this, &o);
	}
	inline ComponentStaticCollisionShapeName(ComponentStaticCollisionShapeName &o)
		: shapeName(o.shapeName)
	{
// 		LOG_INFO("\t\tctor2_copy \t%p (%p)", this, &o);
	}
	inline ComponentStaticCollisionShapeName(ComponentStaticCollisionShapeName &&o)
		: shapeName(std::move(o.shapeName))
	{
// 		LOG_INFO("\t\tctor_move \t%p (%p)", this, &o);
	}

	inline ComponentStaticCollisionShapeName &operator=(const ComponentStaticCollisionShapeName &o)
	{
// 		LOG_INFO("\t\t\tcopy \t\t%p (%p)", this, &o);
		shapeName = o.shapeName;
		return *this;
	}
	inline ComponentStaticCollisionShapeName &operator=(ComponentStaticCollisionShapeName &o)
	{
// 		LOG_INFO("\t\t\t\tcopy2 \t\t%p (%p)", this, &o);
		shapeName = o.shapeName;
		return *this;
	}
	inline ComponentStaticCollisionShapeName &operator=(ComponentStaticCollisionShapeName &&o)
	{
// 		LOG_INFO("\t\t\tmove \t\t%p (%p)", this, &o);
		shapeName = o.shapeName;
		return *this;
	}
};

struct ComponentCharacterSheet {
	float useRange = 4.0f;

	template <typename S> S &__ByteStream_op(S &s)
	{
		s.op(useRange);
		return s;
	}
};
