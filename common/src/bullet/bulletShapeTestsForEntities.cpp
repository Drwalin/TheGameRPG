#include <bullet/LinearMath/btVector3.h>
#include <bullet/btBulletCollisionCommon.h>
#include <bullet/BulletDynamics/Character/btKinematicCharacterController.h>

#include "../../include/CollisionWorld.hpp"
#include "../../include/GlmBullet.hpp"

#if not __unix__
#define M_PI 3.141592653589793
#else
#endif

size_t
CollisionWorld::TestForEntitiesAABB(glm::vec3 min, glm::vec3 max,
									std::vector<uint64_t> *testedEntityIds,
									int32_t filter) const
{
	std::vector<btCollisionObject *> objects;
	GetObjectsInAABB(min, max, filter, &objects);
	testedEntityIds->reserve(testedEntityIds->size() + objects.size());
	size_t count = 0;
	for (const btCollisionObject *o : objects) {
		uint64_t entityId = GetObjectEntityID(o);
		if (entityId) {
			++count;
			testedEntityIds->push_back(entityId);
		}
	}
	return count;
}

size_t CollisionWorld::TestForEntitiesBox(
	glm::vec3 center, glm::vec3 halfExtents, glm::quat rotation,
	std::vector<uint64_t> *testedEntityIds, int32_t filter) const
{
	btBoxShape shape(ToBullet(halfExtents));
	btTransform trans(ToBullet(rotation), ToBullet(center));
	return InternalTestConvexShapeForEntities(&shape, trans, testedEntityIds,
											  filter);
}

size_t
CollisionWorld::TestForEntitiesSphere(glm::vec3 center, float radius,
									  std::vector<uint64_t> *testedEntityIds,
									  int32_t filter) const
{
	btSphereShape shape(radius);
	btTransform trans(btMatrix3x3(), ToBullet(center));
	return InternalTestConvexShapeForEntities(&shape, trans, testedEntityIds,
											  filter);
}

size_t CollisionWorld::TestForEntitiesCylinder(
	glm::vec3 centerBottom, float radius, float height,
	std::vector<uint64_t> *testedEntityIds, int32_t filter) const
{
	btCylinderShape shape(btVector3(radius, height / 2.0, radius));
	btTransform trans(btMatrix3x3(),
					  ToBullet(centerBottom + glm::vec3{0, height / 2.0, 0}));
	return InternalTestConvexShapeForEntities(&shape, trans, testedEntityIds,
											  filter);
}

/*
size_t CollisionWorld::TestForEntitiesCone(
	glm::vec3 peak, glm::vec3 axis, float radius, float height,
	std::vector<uint64_t> *testedEntityIds, int32_t filter) const
{
	LOG_FATAL("CollisionWorld::TestForEntitiesCone not implemented.");
	return 0;
}
*/

size_t CollisionWorld::InternalTestConvexShapeForEntities(
	btConvexShape *shape, btTransform &trans,
	std::vector<uint64_t> *testedEntityIds, int32_t filter) const
{
	btVector3 min, max;
	shape->getAabb(trans, min, max);

	std::vector<btCollisionObject *> objects;
	GetObjectsInAABB(ToGlm(min), ToGlm(max), filter, &objects);
	testedEntityIds->reserve(testedEntityIds->size() + objects.size());

	class Callback : public btCollisionWorld::ConvexResultCallback
	{
	public:
		bool hasHit = false;

		Callback() {}
		virtual ~Callback() {}
		virtual bool needsCollision(btBroadphaseProxy *proxy0) const override
		{
			return true;
		}

		virtual btScalar
		addSingleResult(btCollisionWorld::LocalConvexResult &convexResult,
						bool normalInWorldSpace) override
		{
			hasHit = true;
			return 0;
		}
	};

	size_t count = 0;
	for (btCollisionObject *o : objects) {
		uint64_t entityId = GetObjectEntityID(o);
		if (entityId) {
			Callback cb;
			btCollisionWorld::objectQuerySingle(
				shape, trans, trans, o, o->getCollisionShape(),
				o->getWorldTransform(), cb, 0.01);
			if (cb.hasHit) {
				++count;
				testedEntityIds->push_back(entityId);
			}
		}
	}
	return count;
}
