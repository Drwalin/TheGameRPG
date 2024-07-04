#pragma once

#include <vector>

#include <glm/glm.hpp>
#include <bullet/LinearMath/btVector3.h>
#include <bullet/btBulletCollisionCommon.h>
#include <bullet/BulletDynamics/Character/btKinematicCharacterController.h>

#include <icon7/Debug.hpp>

#include "../../include/GlmBullet.hpp"

#include "../../include/CollisionWorld.hpp"

class BroadphaseAabbAgregate : public btBroadphaseAabbCallback
{
public:
	int32_t filter;
	std::vector<btCollisionObject *> objects;
	BroadphaseAabbAgregate(int32_t filter) : filter(filter) {}
	virtual ~BroadphaseAabbAgregate() {}
	virtual bool process(const btBroadphaseProxy *proxy) override
	{
		if (proxy->m_collisionFilterGroup & filter) {
			objects.emplace_back((btCollisionObject *)proxy->m_clientObject);
		}
		return true;
	}
};

class ClosestRayResultNotMe : public btCollisionWorld::ClosestRayResultCallback
{
public:
	ClosestRayResultNotMe(btCollisionObject *me, glm::vec3 start, glm::vec3 end)
		: btCollisionWorld::ClosestRayResultCallback(ToBullet(start),
													 ToBullet(end))
	{
		m_me = me;
	}

	virtual btScalar
	addSingleResult(btCollisionWorld::LocalRayResult &rayResult,
					bool normalInWorldSpace)
	{
		if (rayResult.m_collisionObject == m_me)
			return 1.0;

		return ClosestRayResultCallback::addSingleResult(rayResult,
														 normalInWorldSpace);
	}

protected:
	btCollisionObject *m_me;
};

class ManifoldResult : public btManifoldResult
{
public:
	std::vector<CollisionWorld::Contact> *contacts;
	btCollisionObject *object;
	bool has = false;
	ManifoldResult(const btCollisionObjectWrapper *body0Wrap,
				   const btCollisionObjectWrapper *body1Wrap,
				   std::vector<CollisionWorld::Contact> *contacts)
		: btManifoldResult(body0Wrap, body1Wrap), contacts(contacts)
	{
	}
	virtual void addContactPoint(const btVector3 &normalOnBInWorld,
								 const btVector3 &pointInWorld,
								 btScalar depth) override
	{
		btCollisionShape *s = object->getCollisionShape();
		btSphereShape *shape = dynamic_cast<btSphereShape *>(s);
		if (shape == nullptr) {
			LOG_FATAL("Currently, only btSphereShape is supported.");
			return;
		}
		float radius = shape->getRadius();
		btVector3 origin = object->getWorldTransform().getOrigin();
		btVector3 d = pointInWorld - origin;
		if (d.length2() > radius * radius) {
			return;
		}

		LOG_DEBUG("depth:%f    origin:%f     hitpoint:%f", depth, origin.y(),
				  pointInWorld.y());

		contacts->push_back({ToGlm(normalOnBInWorld),
							 ToGlm(pointInWorld),
							 fabs(depth),
							 {0, 0, 0},
							 0,
							 {0, 0, 0}});
		has = true;
	}
};
