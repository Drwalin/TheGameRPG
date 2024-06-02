#pragma once

#include <bullet/LinearMath/btVector3.h>
#include <bullet/btBulletCollisionCommon.h>
#include <glm/glm.hpp>
#include <glm/ext/quaternion_float.hpp>
#include <glm/ext/quaternion_common.hpp>

inline glm::vec3 ToGlm(btVector3 v) { return {v.x(), v.y(), v.z()}; }

inline btVector3 ToBullet(glm::vec3 v) { return {v.x, v.y, v.z}; }

inline glm::quat ToGlm(btQuaternion q) { return {q.w(), q.x(), q.y(), q.z()}; }

inline btQuaternion ToBullet(glm::quat q) { return {q.w, q.x, q.y, q.z}; }
