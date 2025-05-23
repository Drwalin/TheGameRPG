#include "../../../thirdparty/bullet/src/BulletCollision/CollisionDispatch/btCollisionWorld.h"
#include "../../../thirdparty/bullet/src/BulletCollision/CollisionShapes/btSphereShape.h"
#include "../../../thirdparty/bullet/src/BulletCollision/CollisionShapes/btBoxShape.h"
#include "../../../thirdparty/bullet/src/BulletCollision/CollisionShapes/btCylinderShape.h"

#include "../../thirdparty/Collision3D/SpatialPartitioning/glm/glm/ext/vector_float3.hpp"

#include <icon7/Debug.hpp>

#include "../../include/CollisionWorld.hpp"
#include "../../include/Realm.hpp"
#include "../../include/GlmBullet.hpp"

#if not __unix__
#define M_PI 3.141592653589793
#else
#endif

size_t
CollisionWorld::TestForEntitiesAABB(glm::vec3 min, glm::vec3 max,
									std::vector<uint64_t> *testedEntityIds,
									CollisionFilter filter) const
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

extern size_t CollisionWorld::TestForEntitiesBox(
	glm::vec3 center, glm::vec3 halfExtents, glm::quat rotation,
	std::vector<uint64_t> *testedEntityIds, CollisionFilter filter) const
{
	btBoxShape shape(ToBullet(halfExtents));
	btTransform trans(ToBullet(rotation), ToBullet(center));
	return InternalTestConvexShapeForEntities(&shape, trans, testedEntityIds,
											  filter);
}

extern size_t
CollisionWorld::TestForEntitiesSphere(glm::vec3 center, float radius,
									  std::vector<uint64_t> *testedEntityIds,
									  CollisionFilter filter) const
{
	btSphereShape shape(radius);
	btTransform trans(btMatrix3x3::getIdentity(), ToBullet(center));
	return InternalTestConvexShapeForEntities(&shape, trans, testedEntityIds,
											  filter);
}

extern size_t CollisionWorld::TestForEntitiesCylinder(
	glm::vec3 centerBottom, float radius, float height,
	std::vector<uint64_t> *testedEntityIds, CollisionFilter filter) const
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
	std::vector<uint64_t> *testedEntityIds, CollisionFilter filter) const
{
	LOG_FATAL("CollisionWorld::TestForEntitiesCone not implemented.");
	return 0;
}
*/

size_t CollisionWorld::InternalTestConvexShapeForEntities(
	btConvexShape *shape, btTransform &trans,
	std::vector<uint64_t> *testedEntityIds, CollisionFilter filter) const
{
	btCollisionObject obj;
	obj.setCollisionShape(shape);
	obj.setWorldTransform(trans);
	collisionWorld->addCollisionObject(&obj, filter, filter);
	collisionWorld->updateSingleAabb(&obj);

	int32_t _filter = 0;
	if (filter & FILTER_CHARACTER) {
		_filter |= btBroadphaseProxy::CharacterFilter;
	} else if (filter & FILTER_TERRAIN) {
		_filter |= btBroadphaseProxy::StaticFilter;
	} else if (filter & FILTER_TRIGGER) {
		_filter |= btBroadphaseProxy::CollisionFilterGroups::SensorTrigger;
	}

	struct _ContactResultCallback
		: public btCollisionWorld::ContactResultCallback {
		std::vector<uint64_t> *objs;
		bool hasHit = false;
		btCollisionObject *self;
		CollisionWorld *cw;
		int32_t filter;
		int count = 0;

		virtual bool needsCollision(btBroadphaseProxy *proxy) const override
		{
			return proxy->m_collisionFilterGroup & filter;
		}

		virtual btScalar
		addSingleResult(btManifoldPoint &cp,
						const btCollisionObjectWrapper *colObj0Wrap,
						int partId0, int index0,
						const btCollisionObjectWrapper *colObj1Wrap,
						int partId1, int index1) override
		{
			uint64_t id = 0;
			if (colObj0Wrap->getCollisionObject() == self) {
				id = CollisionWorld::GetObjectEntityID(
					colObj1Wrap->getCollisionObject());
			} else {
				id = CollisionWorld::GetObjectEntityID(
					colObj0Wrap->getCollisionObject());
			}
			if (id) {
				if (hasHit) {
					if (objs->back() == id) {
						return 0;
					}
				}
				if (cw->realm->Entity(id).is_alive()) {
					hasHit = true;
					objs->push_back(id);
					count++;
				}
			}
			return 0.0;
		}
	};
	_ContactResultCallback _cb;
	_cb.objs = testedEntityIds;
	_cb.self = &obj;
	_cb.cw = (CollisionWorld *)this;
	_cb.filter = _filter;

	collisionWorld->contactTest(&obj, _cb);

	collisionWorld->removeCollisionObject(&obj);
	return _cb.count;
}
