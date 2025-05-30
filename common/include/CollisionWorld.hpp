#pragma once

#include <vector>

#include "../../thirdparty/Collision3D/SpatialPartitioning/glm/glm/ext/vector_float3.hpp"
#include "../../thirdparty/Collision3D/SpatialPartitioning/glm/glm/ext/quaternion_float.hpp"

#include "CollisionFilters.hpp"
#include "ForwardDeclarations.hpp"

class CollisionWorld
{
public:
	CollisionWorld(Realm *realm);
	~CollisionWorld();

	void Clear();

	void Debug() const;

	void StartEpoch();
	void EndEpoch();

	static uint64_t GetObjectEntityID(const btCollisionObject *object);

	btCollisionShape *CreateBtShape(const __InnerShape &shape) const;

	void OnStaticCollisionShape(flecs::entity entity,
								const ComponentCollisionShape &shape,
								const ComponentStaticTransform &transform);
	void OnAddEntity(flecs::entity entity, const ComponentShape &shape,
					 glm::vec3 pos);

	void UpdateEntityBvh_(const ComponentBulletCollisionObject obj,
						  ComponentShape shape, glm::vec3 pos);
	void UpdateEntityBvh(flecs::entity entity, ComponentShape shape,
						 glm::vec3 pos);

	void EntitySetTransform(const ComponentBulletCollisionObject obj,
							const ComponentStaticTransform &transform);

	// returns true if any collision happens
	bool TestCollisionMovementRays(ComponentShape shape, glm::vec3 start,
								   glm::vec3 end,
								   glm::vec3 *finalCorrectedPosition,
								   bool *isOnGround, glm::vec3 *normal,
								   int horizontalRaysCountInMovementDirection,
								   float stepHeight, float minNormalYcomponent,
								   int horizontalFromCenterCorrectionRaysCount,
								   float verticalRayTestToGroundDistance) const;
	bool TestCollisionMovement(ComponentShape shape, glm::vec3 start,
							   glm::vec3 end, glm::vec3 *finalCorrectedPosition,
							   bool *isOnGround, glm::vec3 *normal,
							   int approximationSpheresAmount, float stepHeight,
							   float minNormalYcomponent,
							   float maxDistancePerIteration) const;

	bool TestIsOnGround(glm::vec3 pos, glm::vec3 *groundPoint,
						glm::vec3 *normal, float stepHeight,
						float minNormalYcomponent) const;

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

	size_t TestForEntitiesAABB(glm::vec3 min, glm::vec3 max,
							   std::vector<uint64_t> *testedEntityIds,
							   CollisionFilter filter) const;
	size_t TestForEntitiesBox(glm::vec3 center, glm::vec3 halfExtents,
							  glm::quat rotation,
							  std::vector<uint64_t> *testedEntityIds,
							  CollisionFilter filter) const;
	size_t TestForEntitiesSphere(glm::vec3 center, float radius,
								 std::vector<uint64_t> *testedEntityIds,
								 CollisionFilter filter) const;
	size_t TestForEntitiesCylinder(glm::vec3 centerBottom, float radius,
								   float height,
								   std::vector<uint64_t> *testedEntityIds,
								   CollisionFilter filter) const;
	size_t TestForEntitiesCone(glm::vec3 peak, glm::vec3 axis, float radius,
							   float height,
							   std::vector<uint64_t> *testedEntityIds,
							   CollisionFilter filter) const;

	void TriggerTestBoxForCharacters(flecs::entity entity,
									 std::vector<uint64_t> &entities);

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

	size_t InternalTestConvexShapeForEntities(
		class btConvexShape *shape, class btTransform &trans,
		std::vector<uint64_t> *testedEntityIds, CollisionFilter filter) const;

private:
	CollisionWorld(CollisionWorld &) = delete;
	CollisionWorld(CollisionWorld &&) = delete;
	CollisionWorld(const CollisionWorld &) = delete;
	CollisionWorld &operator=(CollisionWorld &) = delete;
	CollisionWorld &operator=(CollisionWorld &&) = delete;
	CollisionWorld &operator=(const CollisionWorld &) = delete;

	friend class BroadphaseAabbAgregate;

private:

private:
	void RemoveAndDestroyCollisionObject(btCollisionObject *object);
	void DestroyCollisionShape(btCollisionShape *shape);
	class btCollisionObject *AllocateNewCollisionObject();

	class btBroadphaseInterface *broadphase;
	class btDefaultCollisionConfiguration *collisionConfiguration;
	class btCollisionDispatcher *dispatcher;
	class btCollisionWorld *collisionWorld;

	class Realm *realm;

	int dynamicUpdateCounter = 0;
};

