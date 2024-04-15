#pragma once
#include <glm/glm.hpp>

#include <godot_cpp/classes/node3d.hpp>

inline godot::Vector3 ToGodot(glm::vec3 vec) { return {vec.x, vec.y, vec.z}; }

inline glm::vec3 ToGlm(godot::Vector3 vec) { return {vec.x, vec.y, vec.z}; }

inline godot::Vector2 ToGodot(glm::vec2 vec) { return {vec.x, vec.y}; }

inline glm::vec2 ToGlm(godot::Vector2 vec) { return {vec.x, vec.y}; }
