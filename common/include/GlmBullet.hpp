#pragma once

#include <bullet/LinearMath/btVector3.h>
#include <bullet/btBulletCollisionCommon.h>
#include <glm/glm.hpp>

inline glm::vec3 ToGlm(btVector3 v) { return {v.x(), v.y(), v.z()}; }

inline btVector3 ToBullet(glm::vec3 v) { return {v.x, v.y, v.z}; }
