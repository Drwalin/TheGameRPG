#pragma once

#include <string>

#include "CollisionWorld.hpp"

struct CollisionLoader
{
	TerrainCollisionData collisionData;

	bool LoadOBJ_Cpp(std::string fileName);
	bool LoadOBJ(std::string fileName);
};
