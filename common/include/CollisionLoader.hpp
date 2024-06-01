#pragma once

#include <string>

#include "CollisionWorld.hpp"

struct CollisionLoader {
	TerrainCollisionData collisionData;

	bool LoadOBJ(std::string fileName);
};
