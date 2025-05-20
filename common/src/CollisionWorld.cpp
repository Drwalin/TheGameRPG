#include <icon7/Debug.hpp>

#include "../../thirdparty/bullet/src/LinearMath/btVector3.h"
#include "../../thirdparty/bullet/src/LinearMath/btTransform.h"
#include "../../thirdparty/bullet/src/BulletCollision/BroadphaseCollision/btDbvtBroadphase.h"
#include "../../thirdparty/bullet/src/BulletCollision/CollisionDispatch/btCollisionWorld.h"
#include "../../thirdparty/bullet/src/BulletCollision/CollisionDispatch/btDefaultCollisionConfiguration.h"
#include "../../thirdparty/bullet/src/BulletCollision/CollisionShapes/btSphereShape.h"
#include "../../thirdparty/bullet/src/BulletCollision/CollisionShapes/btBoxShape.h"
#include "../../thirdparty/bullet/src/BulletCollision/CollisionShapes/btCylinderShape.h"
#include "../../thirdparty/bullet/src/BulletCollision/CollisionShapes/btCapsuleShape.h"
#include "../../thirdparty/bullet/src/BulletCollision/CollisionShapes/btCompoundShape.h"
#include "../../thirdparty/bullet/src/BulletCollision/CollisionShapes/btHeightfieldTerrainShape.h"

#include "../include/GlmBullet.hpp"
#include "../include/EntityComponents.hpp"
#include "../include/Realm.hpp"

#include "bullet/BulletPhysicsCallbacks.hpp"
#include "../include/CollisionWorld.hpp"

uint64_t CollisionWorld::GetObjectEntityID(const btCollisionObject *object)
{
	return ((uint64_t)(object->getUserIndex2())) |
		   (((uint64_t)(object->getUserIndex3())) << 32);
}

namespace fdkjsfldksfjldkf
{
extern auto ____dsafjkdlsfjklsdafjkldsjafkldj0()
{
	return &CollisionWorld::TestForEntitiesBox;
}
extern auto ____dsafjkdlsfjklsdafjkldsjafkldj1()
{
	return &CollisionWorld::TestForEntitiesSphere;
}
extern auto ____dsafjkdlsfjklsdafjkldsjafkldj2()
{
	return &CollisionWorld::TestForEntitiesCylinder;
}
extern auto ____dsafjkdlsfjklsdafjkldsjafkldj3()
{
	return &CollisionWorld::RayTestFirstHit;
}
extern auto ____dsafjkdlsfjklsdafjkldsjafkldj4()
{
	return &CollisionWorld::RayTestFirstHitTerrain;
}
extern auto ____dsafjkdlsfjklsdafjkldsjafkldj5()
{
	return &CollisionWorld::RayTestFirstHitTerrainVector;
}
extern auto ____dsafjkdlsfjklsdafjkldsjafkldj6()
{
	return &CollisionWorld::TestCollisionMovementRays;
}
} // namespace fdkjsfldksfjldkf

CollisionWorld::CollisionWorld(Realm *realm)
{
	// TODO: Replace this code with some proper *.so compilation configuration
	//       that even more forces to export all symbols than ENABLE_EXPORTS
	this->realm = realm;
	broadphase = new btDbvtBroadphase();
	collisionConfiguration = new btDefaultCollisionConfiguration();
	dispatcher = new btCollisionDispatcher(collisionConfiguration);
	collisionWorld =
		new btCollisionWorld(dispatcher, broadphase, collisionConfiguration);
}

CollisionWorld::~CollisionWorld()
{
	Clear();
	delete collisionWorld;
	delete dispatcher;
	delete collisionConfiguration;
	delete broadphase;
}

void CollisionWorld::Clear()
{
	while (collisionWorld->getNumCollisionObjects() != 0) {
		btCollisionObject *object =
			collisionWorld->getCollisionObjectArray()[0];
		RemoveAndDestroyCollisionObject(object);
	}
}

void CollisionWorld::DestroyCollisionShape(btCollisionShape *shape)
{
	if (shape == nullptr) {
		return;
	}

	if (auto *s = dynamic_cast<btCylinderShape *>(shape)) {
		delete s;
	} else if (auto *s = dynamic_cast<btBoxShape *>(shape)) {
		delete s;
	} else if (auto *s = dynamic_cast<btCapsuleShape *>(shape)) {
		delete s;
	} else if (auto *s = dynamic_cast<btSphereShape *>(shape)) {
		delete s;
	} else if (auto *s = dynamic_cast<btCompoundShape *>(shape)) {
		for (int i = 0; i < s->getNumChildShapes(); ++i) {
			DestroyCollisionShape(s->getChildShape(i));
		}
		delete s;
	} else if (auto *s = dynamic_cast<btHeightfieldTerrainShape *>(shape)) {
		delete s;
	} else {
		LOG_FATAL("Unknown btCollisionShape type: `%s`", shape->getName());
	}
}

void CollisionWorld::RemoveAndDestroyCollisionObject(btCollisionObject *object)
{
	btCollisionShape *shape = object->getCollisionShape();
	collisionWorld->removeCollisionObject(object);
	delete object;
	DestroyCollisionShape(shape);
}

btCollisionObject *CollisionWorld::AllocateNewCollisionObject()
{
	btCollisionObject *object = new btCollisionObject();
	object->setUserIndex(0);
	object->setUserIndex2(0);
	object->setUserIndex3(0);

	return object;
}

btCollisionShape *CollisionWorld::CreateBtShape(const __InnerShape &shape) const
{
	switch (shape.type) {
	case __InnerShape::VERTBOX: {
		LOG_FATAL("vertbox");
		auto s = std::get<Collision3D::VertBox>(shape.shape);
		btCompoundShape *cs = new btCompoundShape(false, 1);
		btTransform trans{ToBullet(shape.trans.rot), ToBullet(shape.trans.pos)};
		btBoxShape *bs = new btBoxShape(ToBullet(s.halfExtents));
		cs->addChildShape(trans, bs);
		cs->setLocalScaling(ToBullet(shape.trans.scale));
		return cs;
	} break;
	case __InnerShape::CYLINDER: {
		LOG_FATAL("cylinder");
		auto s = std::get<Collision3D::Cylinder>(shape.shape);
		btCompoundShape *cs = new btCompoundShape(false, 1);
		btTransform trans{ToBullet(shape.trans.rot), ToBullet(shape.trans.pos)};
		btCylinderShape *bs =
			new btCylinderShape({s.radius, s.height / 2.0f, s.radius});
		cs->addChildShape(trans, bs);
		cs->setLocalScaling(ToBullet(shape.trans.scale));
		return cs;
	} break;
	case __InnerShape::HEIGHTMAP: {
		LOG_FATAL("heightmap");
		auto s = std::get<Collision3D::HeightMap<float, uint8_t>>(shape.shape);
		btCompoundShape *cs = new btCompoundShape(false, 1);
		btTransform trans{ToBullet(shape.trans.rot), ToBullet(shape.trans.pos)};
		btHeightfieldTerrainShape *bs = new btHeightfieldTerrainShape(
			s.width, s.height, (void *)s.mipmap[0].heights.data(), s.scale.y,
			-10000.0f, 100000.0f, 1, PHY_FLOAT, false);
		cs->addChildShape(trans, bs);
		cs->setLocalScaling(ToBullet(shape.trans.scale *
									 glm::vec3{s.scale.x, 1.0f, s.scale.z}));
		return cs;
	} break;
	case __InnerShape::COMPOUND_SHAPE: {
		LOG_FATAL("compound start");
		auto s = std::get<CompoundShape>(shape.shape);
		btCompoundShape *cs = new btCompoundShape(false, s.shapes->size());
		btTransform trans{ToBullet(shape.trans.rot), ToBullet(shape.trans.pos)};
		cs->setLocalScaling(ToBullet(shape.trans.scale));
		for (const auto &ss : *s.shapes) {
			cs->addChildShape(trans, CreateBtShape(ss));
		}
		LOG_FATAL("compound   end");
		return cs;
	} break;
	default:
		LOG_FATAL("Unknown collision shape type: %i", (int)shape.type);
		return nullptr;
	}
}

void CollisionWorld::OnStaticCollisionShape(
	flecs::entity entity, const ComponentCollisionShape &shape,
	const ComponentStaticTransform &transform)
{
	btCollisionShape *btShape = CreateBtShape(shape.shape);
	if (btShape) {
		btShape->setLocalScaling(btShape->getLocalScaling() *
								 ToBullet(transform.scale));
		btCollisionObject *object = AllocateNewCollisionObject();
		object->setCollisionShape(btShape);
		object->setWorldTransform(
			btTransform(ToBullet(transform.rot), ToBullet(transform.pos)));
		object->setUserIndex(FILTER_TERRAIN);
		object->setUserIndex2(((uint32_t)(entity.id())) & 0xFFFFFFFF);
		object->setUserIndex3(((uint32_t)(entity.id() >> 32)) & 0xFFFFFFFF);
		collisionWorld->addCollisionObject(object,
										   btBroadphaseProxy::StaticFilter);
		collisionWorld->updateSingleAabb(object);
		((btDbvtBroadphase *)broadphase)->m_sets[0].optimizeIncremental(1);
		((btDbvtBroadphase *)broadphase)->m_sets[1].optimizeIncremental(1);

		if (entity.has<ComponentBulletCollisionObject>()) {
			ComponentBulletCollisionObject *obj =
				(ComponentBulletCollisionObject *)
					entity.get<ComponentBulletCollisionObject>();
			if (obj->object) {
				RemoveAndDestroyCollisionObject(obj->object);
			}
			obj->object = object;
		} else {
			entity.set<ComponentBulletCollisionObject>({object});
		}
	} else {
		LOG_ERROR("Failed to construct btCollisionShape from "
				 "ComponentCollisionShape");
	}
}

void CollisionWorld::OnAddEntity(flecs::entity entity,
								 const ComponentShape &shape, glm::vec3 pos)
{
	pos.y += shape.height * 0.5f;
	btCapsuleShape *_shape =
		new btCapsuleShape(shape.width * 0.5, shape.height);
	btCollisionObject *object = AllocateNewCollisionObject();
	object->setCollisionShape(_shape);
	object->setWorldTransform(
		btTransform(btQuaternion::getIdentity(), ToBullet(pos)));
	object->setUserIndex(FILTER_CHARACTER);
	object->setUserIndex2(((uint32_t)(entity.id())) & 0xFFFFFFFF);
	object->setUserIndex3(((uint32_t)(entity.id() >> 32)) & 0xFFFFFFFF);
	collisionWorld->addCollisionObject(object,
									   btBroadphaseProxy::CharacterFilter);
	collisionWorld->updateSingleAabb(object);
	((btDbvtBroadphase *)broadphase)->m_sets[0].optimizeIncremental(1);
	((btDbvtBroadphase *)broadphase)->m_sets[1].optimizeIncremental(1);
	entity.set<ComponentBulletCollisionObject>({object});
}

void CollisionWorld::OnAddTrigger(flecs::entity entity,
								  const ComponentStaticTransform &transform)
{
	btBoxShape *_shape = new btBoxShape({0.5, 0.5, 0.5});
	btCollisionObject *object = AllocateNewCollisionObject();
	object->setCollisionShape(_shape);
	object->setUserIndex(FILTER_TRIGGER);
	object->setUserIndex2(((uint32_t)(entity.id())) & 0xFFFFFFFF);
	object->setUserIndex3(((uint32_t)(entity.id() >> 32)) & 0xFFFFFFFF);
	collisionWorld->addCollisionObject(
		object, btBroadphaseProxy::CollisionFilterGroups::SensorTrigger);
	ComponentBulletCollisionObject obj{object};
	EntitySetTransform(obj, transform);
	entity.set<ComponentBulletCollisionObject>(obj);
}

void CollisionWorld::UpdateEntityBvh_(const ComponentBulletCollisionObject obj,
									  ComponentShape shape, glm::vec3 pos)
{
	pos.y += shape.height * 0.5f;
	obj.object->setWorldTransform(
		btTransform(btQuaternion::getIdentity(), ToBullet(pos)));
	collisionWorld->updateSingleAabb(obj.object);
	if (((++dynamicUpdateCounter) & 0x1F) == 13) {
		((btDbvtBroadphase *)broadphase)->m_sets[0].optimizeIncremental(1);
		((btDbvtBroadphase *)broadphase)->m_sets[1].optimizeIncremental(1);
	}
}

void CollisionWorld::UpdateEntityBvh(flecs::entity entity, ComponentShape shape,
									 glm::vec3 pos)
{
	auto obj = entity.get<ComponentBulletCollisionObject>();
	if (obj) {
		UpdateEntityBvh_(*obj, shape, pos);
	}
}

void CollisionWorld::EntitySetTransform(
	const ComponentBulletCollisionObject obj,
	const ComponentStaticTransform &transform)
{
	obj.object->setWorldTransform(
		btTransform(ToBullet(transform.rot), ToBullet(transform.pos)));
	obj.object->getCollisionShape()->setLocalScaling(ToBullet(transform.scale));
	collisionWorld->updateSingleAabb(obj.object);
	((btDbvtBroadphase *)broadphase)->m_sets[0].optimizeIncremental(1);
	((btDbvtBroadphase *)broadphase)->m_sets[1].optimizeIncremental(1);
}

void CollisionWorld::GetObjectsInAABB(
	glm::vec3 aabbMin, glm::vec3 aabbMax, int filter,
	std::vector<btCollisionObject *> *objects) const
{
	int f = 0;
	if (filter & FILTER_TERRAIN) {
		f |= btBroadphaseProxy::StaticFilter;
	}
	if (filter & FILTER_CHARACTER) {
		f |= btBroadphaseProxy::CharacterFilter;
	}
	if (filter & FILTER_TRIGGER) {
		f |= btBroadphaseProxy::SensorTrigger;
	}
	BroadphaseAabbAgregate broadphaseCallback(f);
	std::swap(*objects, broadphaseCallback.objects);

	broadphase->aabbTest(ToBullet(aabbMin), ToBullet(aabbMax),
						 broadphaseCallback);
	std::swap(*objects, broadphaseCallback.objects);
}

void CollisionWorld::StartEpoch()
{
	((btDbvtBroadphase *)broadphase)->m_sets[0].optimizeIncremental(10);
	((btDbvtBroadphase *)broadphase)->m_sets[1].optimizeIncremental(10);
	/*
	collisionWorld->setForceUpdateAllAabbs(true);
	collisionWorld->updateAabbs();
	collisionWorld->computeOverlappingPairs();
	collisionWorld->performDiscreteCollisionDetection();
	*/
}

void CollisionWorld::EndEpoch() {}

void CollisionWorld::Debug() const
{
	auto objs = collisionWorld->getCollisionObjectArray();
	for (int i = 0; i < objs.size(); ++i) {
		auto o = objs.at(i);

		auto shape = o->getCollisionShape();

		if (dynamic_cast<btCylinderShape *>(shape)) {
			LOG_INFO("shape cylinder");
		} else if (dynamic_cast<btCapsuleShape *>(shape)) {
			LOG_INFO("shape capsule");
		} else if (dynamic_cast<btSphereShape *>(shape)) {
			LOG_INFO("shape sphere");
		} else if (dynamic_cast<btBoxShape *>(shape)) {
			LOG_INFO("shape box");
		} else if (dynamic_cast<btHeightfieldTerrainShape *>(shape)) {
			LOG_INFO("shape heightmap");
		} else if (dynamic_cast<btCompoundShape *>(shape)) {
			LOG_INFO("shape compound: NOT PRINTING RECURSIVE");
		} else {
			LOG_INFO("Unknown shape used: %s", shape->getName());
		}
	}
}

void CollisionWorld::RegisterObservers(Realm *realm)
{
	auto &ecs = realm->ecs;
	ecs.observer<ComponentShape, ComponentMovementState>()
		.event(flecs::OnAdd)
		.each([this](flecs::entity entity, const ComponentShape &shape,
					 const ComponentMovementState &state) {
			OnAddEntity(entity, shape, state.pos);
		});
	ecs.observer<ComponentBulletCollisionObject>()
		.event(flecs::OnRemove)
		.each(
			[this](flecs::entity entity, ComponentBulletCollisionObject &obj) {
				if (obj.object) {
					RemoveAndDestroyCollisionObject(obj.object);
					obj.object = nullptr;
				}
			});

	ecs.observer<ComponentShape, ComponentMovementState,
				 ComponentBulletCollisionObject>()
		.event(flecs::OnSet)
		.each([this](flecs::entity entity, const ComponentShape &shape,
					 const ComponentMovementState &state,
					 const ComponentBulletCollisionObject &obj) {
			if (entity.has<ComponentMovementState>()) {
				UpdateEntityBvh_(obj, shape, state.pos);
			}
		});
	ecs.observer<ComponentStaticTransform, ComponentBulletCollisionObject>()
		.event(flecs::OnSet)
		.each([this](flecs::entity entity,
					 const ComponentStaticTransform &transform,
					 const ComponentBulletCollisionObject &obj) {
			EntitySetTransform(obj, transform);
		});
	
	ecs.observer<ComponentCollisionShape, ComponentStaticTransform>()
		.event(flecs::OnSet)
		.each(
			[this](flecs::entity entity, const ComponentCollisionShape &shape, const ComponentStaticTransform &transform) {
				OnStaticCollisionShape(entity, shape, transform);
			});
}

int RegisterEntityComponentsCollisionWorld(flecs::world &ecs)
{
	ecs.component<ComponentBulletCollisionObject>();
	return 0;
}
