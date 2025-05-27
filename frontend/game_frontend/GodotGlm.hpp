#pragma once
#include "../../thirdparty/Collision3D/SpatialPartitioning/glm/glm/glm.hpp"
#include "../../thirdparty/Collision3D/SpatialPartitioning/glm/glm/ext/quaternion_float.hpp"
#include "../../thirdparty/Collision3D/SpatialPartitioning/glm/glm/ext/quaternion_common.hpp"

#include <godot_cpp/variant/quaternion.hpp>
#include <godot_cpp/variant/vector2.hpp>
#include <godot_cpp/variant/vector3.hpp>
#include <godot_cpp/variant/transform3d.hpp>

#include "../../thirdparty/Collision3D/include/collision3d/Transform.hpp"
#include "godot_cpp/variant/transform3d.hpp"

inline godot::Vector3 ToGodot(glm::vec3 vec) { return {vec.x, vec.y, vec.z}; }

inline glm::vec3 ToGlm(godot::Vector3 vec) { return {vec.x, vec.y, vec.z}; }

inline godot::Vector2 ToGodot(glm::vec2 vec) { return {vec.x, vec.y}; }

inline glm::vec2 ToGlm(godot::Vector2 vec) { return {vec.x, vec.y}; }

inline glm::quat ToGlm(godot::Quaternion q)
{
	return glm::quat(q.w, q.x, q.y, q.z);
}

inline godot::Quaternion ToGodot(glm::quat q)
{
	return godot::Quaternion(q.x, q.y, q.z, q.w);
}

inline godot::Quaternion ToGodot(Collision3D::Rotation r)
{
	return godot::Quaternion(godot::Vector3{0, 1, 0}, r.ToRadians());
}

inline godot::Transform3D ToGodot(Collision3D::Transform t)
{
	return godot::Transform3D{godot::Basis(ToGodot(t.rot)), ToGodot(t.pos)};
}
