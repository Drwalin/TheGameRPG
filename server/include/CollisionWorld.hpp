#pragma once

#include <cstdio>
#include <cinttypes>

#include <vector>

#include <glm/glm.hpp>

struct EntityShape {
	float height;
	float radius;
};

struct Triangle {
	glm::vec3 a, b, c;
};

struct DynamicObject {
	std::vector<Triangle> triangles;
	uint64_t objectId;
};

class CollisionWorld
{
public:
	bool LoadStaticCollision(const char *filePath);

	void AddDynamicObject(DynamicObject *object);
	void UpdateDynamicObjectBvh(DynamicObject *object);
	void DeleteDynamicObject(DynamicObject *object);

	void AddEntity(uint64_t entityId, EntityShape shape, glm::vec3 pos);
	void UpdateEntityBvh(uint64_t entityId, EntityShape shape, glm::vec3 pos);
	void DeleteEntity(uint64_t entityId);

	// returns true if any collision happens
	bool TestCollisionMovement(EntityShape shape, glm::vec3 start,
							   glm::vec3 end,
							   glm::vec3 *finalCorrectedPosition) const;
	bool RayTestFirstHit(glm::vec3 start, glm::vec3 end, glm::vec3 *hitPosition,
						 glm::vec3 *hitNormal, uint64_t *entityId,
						 uint64_t *dynamicObjectId, float *travelFactor,
						 bool *hasNormal) const;

	// 	bool TestForEntities(class CustomShape &shape, std::vector<uint64_t>
	// *testedEntityIds) const; 	bool TestForCollisionObjects(class CustomShape
	// &shape, std::vector<uint64_t> *testedCollisionObjectIds) const;

private:
};
