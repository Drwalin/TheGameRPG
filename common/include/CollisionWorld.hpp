#pragma once

#include <vector>

#include "../../thirdparty/Collision3D/SpatialPartitioning/include/spatial_partitioning/BroadPhaseBase.hpp"

#include "../../thirdparty/Collision3D/SpatialPartitioning/glm/glm/ext/vector_float3.hpp"

#include "ForwardDeclarations.hpp"

class CollisionWorld_spp
{
private:
	using CallbackAabb = spp::AabbCallback<spp::Aabb, uint32_t, uint32_t, 0>;
	using CallbackRay = spp::RayCallback<spp::Aabb, uint32_t, uint32_t, 0>;
	using CallbackRayFirstHit =
		spp::RayCallbackFirstHit<spp::Aabb, uint32_t, uint32_t, 0>;

public:
	CollisionWorld_spp(Realm *realm);
	~CollisionWorld_spp();

	void Clear();

	void StartEpoch();
	void EndEpoch();

	void OnStaticCollisionShape(flecs::entity entity,
								const ComponentCollisionShape &shape,
								const ComponentStaticTransform &transform);
	void OnAddEntity(flecs::entity entity, const ComponentShape &shape,
					 glm::vec3 pos);

	void EntitySetTransform(flecs::entity entity,
							const ComponentStaticTransform &trans,
							const ComponentCollisionShape &shape);
	void EntitySetTransform(flecs::entity entity, const glm::vec3 &pos,
							const ComponentShape &shape);

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
						 glm::vec3 *hitNormal, flecs::entity *entity,
						 float *travelFactor, uint32_t ignoreEntityId,
						 uint32_t mask) const;
	bool RayTestFirstHitTerrain(glm::vec3 start, glm::vec3 end,
								glm::vec3 *hitPosition, glm::vec3 *hitNormal,
								float *travelFactor,
								flecs::entity *entity) const;
	bool RayTestFirstHitTerrainVector(glm::vec3 start, glm::vec3 toEnd,
									  glm::vec3 *hitPosition,
									  glm::vec3 *hitNormal, float *travelFactor,
									  flecs::entity *entity) const;

	size_t TestForEntitiesAABB(glm::vec3 min, glm::vec3 max,
							   std::vector<flecs::entity> *entities,
							   uint32_t mask) const;

	/*
	size_t TestForEntitiesVertBox(glm::vec3 center, glm::vec3 halfExtents,
								  glm::quat rotation,
								  std::vector<uint32_t> *testedEntityIds,
								  uint32_t mask) const;
	size_t TestForEntitiesSphere(glm::vec3 center, float radius,
								 std::vector<flecs::entity> *testedEntityIds,
								 uint32_t mask) const;
	size_t TestForEntitiesCylinder(glm::vec3 centerBottom, float radius,
								   float height,
								   std::vector<flecs::entity> *testedEntityIds,
								   uint32_t mask) const;
	size_t TestForEntitiesCone(glm::vec3 peak, glm::vec3 axis, float radius,
							   float height,
							   std::vector<flecs::entity> *testedEntityIds,
							   uint32_t mask) const;
	*/

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

	uint32_t GetObjectsInAABB(glm::vec3 aabbMin, glm::vec3 aabbMax,
							  uint32_t mask,
							  std::vector<flecs::entity> *objects) const;
	bool
	RayTestFirstHitWithObjects(glm::vec3 start, glm::vec3 direction,
							   glm::vec3 *hitPosition, glm::vec3 *hitNormal,
							   float *travelFactor,
							   const std::vector<flecs::entity> &objects) const;

private:
	flecs::entity GetAliveEntityGeneration(uint32_t id) const;

private:
	CollisionWorld_spp(CollisionWorld_spp &) = delete;
	CollisionWorld_spp(CollisionWorld_spp &&) = delete;
	CollisionWorld_spp(const CollisionWorld_spp &) = delete;
	CollisionWorld_spp &operator=(CollisionWorld_spp &) = delete;
	CollisionWorld_spp &operator=(CollisionWorld_spp &&) = delete;
	CollisionWorld_spp &operator=(const CollisionWorld_spp &) = delete;

	friend class BroadphaseAabbAgregate;

private:
	spp::BroadphaseBase<spp::Aabb, uint32_t, uint32_t, 0> *broadphase = nullptr;

	class Realm *realm = nullptr;
};

using CollisionWorld = CollisionWorld_spp;
