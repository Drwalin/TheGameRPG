#pragma once

#include <cstdio>
#include <cinttypes>

#include <vector>

#include <glm/glm.hpp>

#ifdef COLLISION_WORLD_IMPL
# include <unordered_map>
# include <bullet/LinearMath/btVector3.h>
# include <bullet/btBulletCollisionCommon.h>
#endif

#include "EntityShape.hpp"

struct TerrainCollisionData
{
	std::vector<glm::vec3> vertices;
	std::vector<uint32_t> indices;
};

class CollisionWorld
{
public:
	struct EntityInfo
	{
		EntityShape shape;
		glm::vec3 pos;
	};
	
	static CollisionWorld *Allocate();
	static void Free(CollisionWorld *world);
	
	
	void LoadStaticCollision(const TerrainCollisionData *data);

	bool AddEntity(uint64_t entityId, EntityShape shape, glm::vec3 pos);
	void UpdateEntityBvh(uint64_t entityId, EntityShape shape, glm::vec3 pos);
	void DeleteEntity(uint64_t entityId);

	// returns true if any collision happens
	bool TestCollisionMovement(EntityShape shape, glm::vec3 start,
							   glm::vec3 end,
							   glm::vec3 *finalCorrectedPosition, bool *isOnGround) const;
	bool RayTestFirstHit(glm::vec3 start, glm::vec3 end, glm::vec3 *hitPosition,
						 glm::vec3 *hitNormal, uint64_t *entityId,
						 float *travelFactor, bool *hasNormal,
						 uint64_t ignoreEntityId) const;
	bool RayTestFirstHitTerrain(glm::vec3 start, glm::vec3 end, glm::vec3 *hitPosition,
						 glm::vec3 *hitNormal, float *travelFactor) const;

	// 	bool TestForEntities(class CustomShape &shape, std::vector<uint64_t>
	// *testedEntityIds) const;

private:
	CollisionWorld();
	~CollisionWorld();
	
	CollisionWorld(CollisionWorld &) = delete;
	CollisionWorld(CollisionWorld &&) = delete;
	CollisionWorld(const CollisionWorld &) = delete;
	CollisionWorld &operator=(CollisionWorld &) = delete;
	CollisionWorld &operator=(CollisionWorld &&) = delete;
	CollisionWorld &operator=(const CollisionWorld &) = delete;
	
private:
#ifdef COLLISION_WORLD_IMPL
	
	inline const static int32_t FILTER_MASK_ALL = -1;
	
	inline const static int32_t FILTER_GROUP_TERRAIN = 1;
	inline const static int32_t FILTER_MASK_TERRAIN = -1;
	
	inline const static int32_t FILTER_GROUP_ENTITY = 2;
	inline const static int32_t FILTER_MASK_ENTITY = -1 & (~2);
	
	void Destroy();
	
	void RemoveAndDestroyCollisionObject(btCollisionObject *object);
	btCollisionObject *AllocateNewCollisionObject();
	
	btDbvtBroadphase *broadphase;
	btDefaultCollisionConfiguration *collisionConfiguration;
	btCollisionDispatcher *dispatcher;
	btCollisionWorld *collisionWorld;
	bool updateWorldBvh;
	
	std::unordered_map<uint64_t, btCollisionObject *> entities;
	
#endif
};
