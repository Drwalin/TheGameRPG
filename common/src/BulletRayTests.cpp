#include <icon7/Debug.hpp>

#include <bullet/LinearMath/btVector3.h>
#include <bullet/btBulletCollisionCommon.h>
#include <bullet/BulletDynamics/Character/btKinematicCharacterController.h>

#include "../include/GlmBullet.hpp"
#include "../include/Realm.hpp"
#include "BulletPhysicsCallbacks.hpp"

#include "../include/CollisionWorld.hpp"

bool CollisionWorld::RayTestFirstHitWithObjects(
	glm::vec3 start, glm::vec3 direction, glm::vec3 *hitPosition,
	glm::vec3 *hitNormal, float *travelFactor,
	const std::vector<btCollisionObject *> &objects) const
{
	glm::vec3 end = start + direction;
	btVector3 _start = ToBullet(start), _end = ToBullet(end);
	btTransform _startT{btQuaternion(), _start}, _endT{btQuaternion(), _end};

	btCollisionWorld::ClosestRayResultCallback callback(_start, _end);

	for (btCollisionObject *object : objects) {
		collisionWorld->rayTestSingle(_startT, _endT, object,
									  object->getCollisionShape(),
									  object->getWorldTransform(), callback);
	}

	if (callback.hasHit() == false) {
		if (travelFactor)
			*travelFactor = 1;
		if (hitNormal)
			*hitNormal = {0, 0, 0};
		if (hitPosition)
			*hitPosition = end;
		if (travelFactor)
			*travelFactor = 1.0;
		return false;
	}

	if (hitPosition) {
		*hitPosition = ToGlm(callback.m_hitPointWorld);
	}
	if (hitNormal) {
		*hitNormal = ToGlm(callback.m_hitNormalWorld);
		*hitNormal = glm::normalize(*hitNormal);
		if (glm::dot(*hitNormal, end - start) > 0) {
			*hitNormal = -*hitNormal;
		}
	}

	if (travelFactor)
		*travelFactor = callback.m_closestHitFraction;

	return true;
}

bool CollisionWorld::RayTestFirstHit(glm::vec3 start, glm::vec3 end,
									 glm::vec3 *hitPosition,
									 glm::vec3 *hitNormal, uint64_t *entityId,
									 float *travelFactor, bool *hasNormal,
									 uint64_t ignoreEntityId) const
{
	auto entity = realm->Entity(ignoreEntityId);
	btCollisionObject *object = nullptr;
	auto obj = entity.get<ComponentBulletCollisionObject>();
	if (obj) {
		object = obj->object;
	}
	ClosestRayResultNotMe cb(object, start, end);
	cb.m_collisionFilterMask = btBroadphaseProxy::AllFilter;
	collisionWorld->rayTest({start.x, start.y, start.z}, {end.x, end.y, end.z},
							cb);
	if (cb.hasHit() == false) {
		if (travelFactor)
			*travelFactor = 1;
		if (hasNormal)
			*hasNormal = false;
		if (entityId)
			*entityId = 0;
		return false;
	}

	if (hitPosition)
		*hitPosition = {cb.m_hitPointWorld.x(), cb.m_hitPointWorld.y(),
						cb.m_hitPointWorld.z()};
	if (hitNormal) {
		*hitNormal = {cb.m_hitNormalWorld.x(), cb.m_hitNormalWorld.y(),
					  cb.m_hitNormalWorld.z()};
		*hitNormal = glm::normalize(*hitNormal);
	}

	const btCollisionObject *hitObject = cb.m_collisionObject;
	if (entityId)
		*entityId = ((uint64_t)(hitObject->getUserIndex2())) |
					(((uint64_t)(hitObject->getUserIndex3())) << 32);

	if (travelFactor)
		*travelFactor = cb.m_closestHitFraction;
	if (hasNormal)
		*hasNormal = ((*entityId) == 0);

	return true;
}

bool CollisionWorld::RayTestFirstHitTerrainVector(glm::vec3 start,
												  glm::vec3 toEnd,
												  glm::vec3 *hitPosition,
												  glm::vec3 *hitNormal,
												  float *travelFactor) const
{
	return RayTestFirstHitTerrain(start, start + toEnd, hitPosition, hitNormal,
								  travelFactor);
}

bool CollisionWorld::RayTestFirstHitTerrain(glm::vec3 start, glm::vec3 end,
											glm::vec3 *hitPosition,
											glm::vec3 *hitNormal,
											float *travelFactor) const
{
	btCollisionWorld::ClosestRayResultCallback cb(ToBullet(start),
												  ToBullet(end));
	cb.m_collisionFilterMask = btBroadphaseProxy::StaticFilter;
	collisionWorld->rayTest(ToBullet(start), ToBullet(end), cb);
	if (cb.hasHit() == false) {
		if (travelFactor)
			*travelFactor = 1;
		if (hitNormal)
			*hitNormal = {0, 0, 0};
		if (hitPosition)
			*hitPosition = end;
		if (travelFactor)
			*travelFactor = 1.0;
		return false;
	}

	if (hitPosition) {
		*hitPosition = ToGlm(cb.m_hitPointWorld);
	}
	if (hitNormal) {
		*hitNormal = ToGlm(cb.m_hitNormalWorld);
		*hitNormal = glm::normalize(*hitNormal);
		if (glm::dot(*hitNormal, end - start) > 0) {
			*hitNormal = -*hitNormal;
		}
	}

	if (travelFactor)
		*travelFactor = cb.m_closestHitFraction;

	return true;
}
