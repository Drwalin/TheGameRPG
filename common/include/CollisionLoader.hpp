#pragma once

#include <string>

#include "CollisionWorld.hpp"

struct CollisionLoader {
	TerrainCollisionData collisionData;

	bool LoadOBJ(std::string fileName);
	void LoadOBJ(const void *fileBuffer, size_t bytes);
	void LoadOBJ(std::istream &stream);
};
