#pragma once

#include <glm/glm.hpp>

#include "godot_cpp/classes/node3d.hpp"

godot::Vector3 ToGodot(glm::vec3 vec)
{
	return {vec.x, vec.y, vec.z};
}
