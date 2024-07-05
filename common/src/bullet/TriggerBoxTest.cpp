#include <glm/glm.hpp>
#include <bullet/LinearMath/btVector3.h>
#include <bullet/btBulletCollisionCommon.h>
#include <bullet/BulletDynamics/Character/btKinematicCharacterController.h>

#include <icon7/Debug.hpp>

#include "../../include/CollisionWorld.hpp"

class GhostTestCallbackResult : public btCollisionWorld::ContactResultCallback
{
public:
	GhostTestCallbackResult() {}
	~GhostTestCallbackResult() {}

	virtual bool needsCollision(btBroadphaseProxy *proxy) const override
	{
		return proxy->m_collisionFilterGroup & m_collisionFilterGroup;
	}

	virtual btScalar addSingleResult(btManifoldPoint &cp,
									 const btCollisionObjectWrapper *w1, int,
									 int, const btCollisionObjectWrapper *w2,
									 int, int) override
	{
		btCollisionObjectWrapper const *w = nullptr;
		if (w1->m_collisionObject == triggerObject) {
			w = w2;
		} else {
			w = w1;
		}

		if (w->m_collisionObject->getUserIndex() | filter) {
			uint64_t entityId =
				((uint64_t)(w->m_collisionObject->getUserIndex2())) |
				(((uint64_t)(w->m_collisionObject->getUserIndex3())) << 32);

			entities->push_back(entityId);
		}

		return 1.0;
	}

public:
	uint64_t triggerEntityId;
	btCollisionObject *triggerObject;

	std::vector<uint64_t> *entities;

	int filter;
};

void CollisionWorld::TriggerTestBoxForCharacters(
	flecs::entity entity, std::vector<uint64_t> &entities)
{
	entities.clear();

	auto obj = entity.get<ComponentBulletCollisionObject>();
	if (obj == nullptr) {
		LOG_ERROR(
			"No component bullet collision object found for trigger entity");
		return;
	}

	GhostTestCallbackResult cb;
	cb.triggerEntityId = entity.id();
	cb.triggerObject = obj->object;

	cb.entities = &entities;

	cb.filter = FILTER_CHARACTER;
	cb.m_collisionFilterGroup = btBroadphaseProxy::CharacterFilter;

	collisionWorld->contactTest(obj->object, cb);
}
