#pragma once

#include <vector>
#include <unordered_map>

#include <glm/glm.hpp>

#include "EntityData.hpp"

struct TerrainCollisionData {
	std::vector<glm::vec3> vertices;
	std::vector<uint32_t> indices;
};

class btCylinderShape;
class btCapsuleShape;
class btSphereShape;
class btBvhTriangleMeshShape;
class btCollisionObject;

class Realm;

class CollisionWorld
{
public:
	CollisionWorld(Realm *realm);
	~CollisionWorld();

	void Clear();

	struct EntityInfo {
		EntityShape shape;
		glm::vec3 pos;
	};

	void Debug() const;

	void LoadStaticCollision(const TerrainCollisionData *data,
							 EntityStaticTransform transform);

	bool AddEntity(uint64_t entityId, EntityShape shape, glm::vec3 pos);
	void UpdateEntityBvh(uint64_t entityId, EntityShape shape, glm::vec3 pos);
	void DeleteEntity(uint64_t entityId);

	// returns true if any collision happens
	bool TestCollisionMovementRays(EntityShape shape, glm::vec3 start,
								   glm::vec3 end,
								   glm::vec3 *finalCorrectedPosition,
								   bool *isOnGround, glm::vec3 *normal,
								   int horizontalRaysCountInMovementDirection,
								   float stepHeight, float minNormalYcomponent,
								   int horizontalFromCenterCorrectionRaysCount,
								   float verticalRayTestToGroundDistance) const;
	bool TestCollisionMovement(EntityShape shape, glm::vec3 start,
							   glm::vec3 end, glm::vec3 *finalCorrectedPosition,
							   bool *isOnGround, glm::vec3 *normal,
							   int approximationSpheresAmount, float stepHeight,
							   float minNormalYcomponent,
							   float maxDistancePerIteration) const;
	bool RayTestFirstHit(glm::vec3 start, glm::vec3 end, glm::vec3 *hitPosition,
						 glm::vec3 *hitNormal, uint64_t *entityId,
						 float *travelFactor, bool *hasNormal,
						 uint64_t ignoreEntityId) const;
	bool RayTestFirstHitTerrain(glm::vec3 start, glm::vec3 end,
								glm::vec3 *hitPosition, glm::vec3 *hitNormal,
								float *travelFactor) const;
	bool RayTestFirstHitTerrainVector(glm::vec3 start, glm::vec3 toEnd,
									  glm::vec3 *hitPosition,
									  glm::vec3 *hitNormal,
									  float *travelFactor) const;

	bool TestIsOnGround(glm::vec3 pos, glm::vec3 *groundPoint,
						glm::vec3 *normal, float stepHeight,
						float minNormalYcomponent) const;

	/*
	bool TestForEntities(class CustomShape &shape,
						 std::vector<uint64_t> *testedEntityIds) const;
	*/

	void RegisterObservers(Realm *realm);

private:
	friend class ManifoldResult;

	struct Contact {
		glm::vec3 normal;
		glm::vec3 point;
		float depth;
		glm::vec3 dir;
		float distance;
		glm::vec3 objectPos;
	};

	/*
	 * length(dir) == 1
	 */
	bool PerformObjectSweep(
		btCollisionObject *object, glm::vec3 start, glm::vec3 dir, float step,
		float maxDistance, const std::vector<btCollisionObject *> &otherObjects,
		std::vector<Contact> *contacts, float *distanceTraveled) const;

	bool
	TestObjectCollision(btCollisionObject *object,
						const std::vector<btCollisionObject *> &otherObjects,
						std::vector<Contact> *contacts) const;

	void FindCorrectTravelDistance(const std::vector<Contact> &contacts,
								   glm::vec3 start,
								   float currentTraveledDistance,
								   float *correctedTravelDistance) const;
	void FindPushoutVector(const std::vector<Contact> &contacts,
						   glm::vec3 position, glm::vec3 *pushoutVector) const;

	void GetObjectsInAABB(glm::vec3 aabbMin, glm::vec3 aabbMax, int filter,
						  std::vector<btCollisionObject *> *objects) const;
	bool RayTestFirstHitWithObjects(
		glm::vec3 start, glm::vec3 direction, glm::vec3 *hitPosition,
		glm::vec3 *hitNormal, float *travelFactor,
		const std::vector<btCollisionObject *> &objects) const;

private:
	CollisionWorld(CollisionWorld &) = delete;
	CollisionWorld(CollisionWorld &&) = delete;
	CollisionWorld(const CollisionWorld &) = delete;
	CollisionWorld &operator=(CollisionWorld &) = delete;
	CollisionWorld &operator=(CollisionWorld &&) = delete;
	CollisionWorld &operator=(const CollisionWorld &) = delete;

	friend class BroadphaseAabbAgregate;

private:
	inline const static int32_t FILTER_ALL = 3;
	inline const static int32_t FILTER_TERRAIN = 1;
	inline const static int32_t FILTER_ENTITY = 2;

	void RemoveAndDestroyCollisionObject(btCollisionObject *object);
	class btCollisionObject *AllocateNewCollisionObject();

	class btSimpleBroadphase *broadphase;
	class btDefaultCollisionConfiguration *collisionConfiguration;
	class btCollisionDispatcher *dispatcher;
	class btCollisionWorld *collisionWorld;

	std::unordered_map<uint64_t, btCollisionObject *> entities;

	class Realm *realm;
};
