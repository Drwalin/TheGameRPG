#pragma once

#include "../../thirdparty/bullet/src/LinearMath/btVector3.h"
#include "../../thirdparty/bullet/src/LinearMath/btQuaternion.h"

#include "../../thirdparty/Collision3D/SpatialPartitioning/glm/glm/ext/vector_float3.hpp"
#include "../../thirdparty/Collision3D/SpatialPartitioning/glm/glm/ext/quaternion_float.hpp"

#include "../../thirdparty/Collision3D/include/collision3d/Rotation.hpp"

inline glm::vec3 ToGlm(btVector3 v) { return {v.x(), v.y(), v.z()}; }

inline btVector3 ToBullet(glm::vec3 v) { return {v.x, v.y, v.z}; }

inline glm::quat ToGlm(btQuaternion q)
{
	return glm::quat(q.w(), q.x(), q.y(), q.z());
}

inline btQuaternion ToBullet(glm::quat q)
{
	return btQuaternion(q.x, q.y, q.z, q.w);
}

inline btQuaternion ToBullet(Collision3D::Rotation r)
{
	return btQuaternion(btVector3{0, 1, 0}, r.ToRadians());
}
