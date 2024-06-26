#include <icon7/Debug.hpp>

#include <bullet/LinearMath/btVector3.h>
#include <bullet/btBulletCollisionCommon.h>
#include <bullet/BulletDynamics/Character/btKinematicCharacterController.h>

#include "../include/GlmBullet.hpp"

#include "../include/Realm.hpp"

#include "BulletPhysicsCallbacks.hpp"
#include "../include/CollisionWorld.hpp"

CollisionWorld::CollisionWorld(Realm *realm)
{
	this->realm = realm;
	broadphase = new btSimpleBroadphase();
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

void CollisionWorld::RemoveAndDestroyCollisionObject(btCollisionObject *object)
{
	btCollisionShape *shape = object->getCollisionShape();

	collisionWorld->removeCollisionObject(object);
	delete object;

	if (shape == nullptr) {
		return;
	}

	if (auto *s = dynamic_cast<btCylinderShape *>(shape)) {
		delete s;
	} else if (auto *s = dynamic_cast<btCapsuleShape *>(shape)) {
		delete s;
	} else if (auto *s = dynamic_cast<btSphereShape *>(shape)) {
		delete s;
	} else if (auto *s = dynamic_cast<btBvhTriangleMeshShape *>(shape)) {
		delete s->getMeshInterface();
		delete s;
	}
}

btCollisionObject *CollisionWorld::AllocateNewCollisionObject()
{
	btCollisionObject *object = new btCollisionObject();
	object->setUserIndex(0);
	object->setUserIndex2(0);
	object->setUserIndex3(0);
	return object;
}

void CollisionWorld::OnStaticCollisionShape(
	flecs::entity entity,
	const ComponentStaticCollisionShapeName &collisionName,
	const ComponentStaticTransform &transform)
{

	TerrainCollisionData data;
	if (realm->GetCollisionShape(collisionName.shapeName, &data)) {
		btTriangleMesh *triangles =
			new btTriangleMesh((data.vertices.size() / 3) > (1 << 15), false);
		triangles->preallocateVertices(data.vertices.size());
		triangles->preallocateIndices(data.indices.size());
		std::vector<uint32_t> map;
		map.resize(data.vertices.size());
		for (int i = 0; i < data.vertices.size(); ++i) {
			const glm::vec3 &v = data.vertices[i];
			map[i] = triangles->findOrAddVertex({v.x, v.y, v.z}, false);
		}
		for (int i = 0; i + 2 < data.indices.size(); i += 3) {
			const uint32_t *idx = &(data.indices[i]);
			triangles->addTriangleIndices(idx[0], idx[1], idx[2]);
		}
		btBvhTriangleMeshShape *shape =
			new btBvhTriangleMeshShape(triangles, false, true);
		shape->buildOptimizedBvh();
		btCollisionObject *object = AllocateNewCollisionObject();
		shape->setLocalScaling(ToBullet(transform.scale));
		object->setCollisionShape(shape);
		object->setWorldTransform(
			btTransform(ToBullet(transform.rot), ToBullet(transform.pos)));
		object->setUserIndex(FILTER_TERRAIN);
		object->setUserIndex2(((uint32_t)(entity.id())) & 0xFFFFFFFF);
		object->setUserIndex3(((uint32_t)(entity.id() >> 32)) & 0xFFFFFFFF);
		collisionWorld->addCollisionObject(object,
										   btBroadphaseProxy::StaticFilter);
		collisionWorld->updateSingleAabb(object);

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
		LOG_WARN("No such static collision shape: `%s`",
				 collisionName.shapeName.c_str());
	}
}

void CollisionWorld::OnAddEntity(flecs::entity entity, ComponentShape shape,
								 glm::vec3 pos)
{
	btCapsuleShape *_shape =
		new btCapsuleShape(shape.width * 0.5, shape.height);
	btCollisionObject *object = AllocateNewCollisionObject();
	object->setCollisionShape(_shape);
	object->setWorldTransform(btTransform(btQuaternion(), ToBullet(pos)));
	object->setUserIndex(FILTER_ENTITY);
	object->setUserIndex2(((uint32_t)(entity.id())) & 0xFFFFFFFF);
	object->setUserIndex3(((uint32_t)(entity.id() >> 32)) & 0xFFFFFFFF);
	collisionWorld->addCollisionObject(object,
									   btBroadphaseProxy::CharacterFilter);
	collisionWorld->updateSingleAabb(object);
	entity.set<ComponentBulletCollisionObject>({object});
}

void CollisionWorld::UpdateEntityBvh(const ComponentBulletCollisionObject obj,
									 ComponentShape shape, glm::vec3 pos)
{
	obj.object->setWorldTransform(btTransform(btQuaternion(), ToBullet(pos)));
	collisionWorld->updateSingleAabb(obj.object);
}

void CollisionWorld::UpdateEntityBvh(flecs::entity entity, ComponentShape shape,
									 glm::vec3 pos)
{
	auto obj = entity.get<ComponentBulletCollisionObject>();
	if (obj) {
		UpdateEntityBvh(*obj, shape, pos);
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
}

void CollisionWorld::GetObjectsInAABB(
	glm::vec3 aabbMin, glm::vec3 aabbMax, int filter,
	std::vector<btCollisionObject *> *objects) const
{
	int f = 0;
	if (filter & FILTER_TERRAIN) {
		f |= btBroadphaseProxy::StaticFilter;
	}
	if (filter & FILTER_ENTITY) {
		f |= btBroadphaseProxy::CharacterFilter;
	}
	BroadphaseAabbAgregate broadphaseCallback(f);
	std::swap(*objects, broadphaseCallback.objects);

	broadphase->aabbTest(ToBullet(aabbMin), ToBullet(aabbMax),
						 broadphaseCallback);
	std::swap(*objects, broadphaseCallback.objects);
}

void CollisionWorld::Debug() const
{
	auto objs = collisionWorld->getCollisionObjectArray();
	for (int i = 0; i < objs.size(); ++i) {
		auto o = objs.at(i);

		auto shape = o->getCollisionShape();

		if (auto *s = dynamic_cast<btCylinderShape *>(shape)) {
			LOG_INFO("shape cylinder");
		} else if (auto *s = dynamic_cast<btCapsuleShape *>(shape)) {
			LOG_INFO("shape capsule");
		} else if (auto *s = dynamic_cast<btSphereShape *>(shape)) {
			LOG_INFO("shape sphere");
		} else if (auto *s = dynamic_cast<btBvhTriangleMeshShape *>(shape)) {
			auto a = o->getWorldTransform().getOrigin();
			LOG_INFO("shape triangles: %f %f %f", a.x(), a.y(), a.z());
			auto m = s->getMeshInterface();
			class Cb : public btInternalTriangleIndexCallback
			{
			public:
				virtual void internalProcessTriangleIndex(btVector3 *triangle,
														  int partId,
														  int triangleIndex)
				{
					processTriangle(triangle, partId, triangleIndex);
				}

				virtual void processTriangle(btVector3 *t, int partId,
											 int triangleIndex)
				{
					LOG_INFO("partId: %i, triangleIndex: %i", partId,
							 triangleIndex);

					LOG_INFO("Triangle: (%f %f %f) (%f %f %f) (%f %f %f)",
							 t[0].x(), t[0].y(), t[0].z(), t[1].x(), t[1].y(),
							 t[1].z(), t[2].x(), t[2].y(), t[2].z());
				}

			} cb;

			m->InternalProcessAllTriangles(&cb, {-100000, -100000, -100000},
										   {100000, 100000, 100000});
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
				UpdateEntityBvh(obj, shape, state.pos);
			}
		});
	ecs.observer<ComponentStaticTransform, ComponentBulletCollisionObject>()
		.event(flecs::OnSet)
		.each([this](flecs::entity entity,
					 const ComponentStaticTransform &transform,
					 const ComponentBulletCollisionObject &obj) {
			EntitySetTransform(obj, transform);
		});
	ecs.observer<ComponentStaticCollisionShapeName>()
		.event(flecs::OnSet)
		.each([this](flecs::entity entity,
					 const ComponentStaticCollisionShapeName &collisionName) {
			auto transform = entity.get<ComponentStaticTransform>();
			if (transform && collisionName.shapeName != "") {
				OnStaticCollisionShape(entity, collisionName, *transform);
			}
		});
	ecs.observer<ComponentStaticCollisionShapeName>()
		.event(flecs::OnAdd)
		.each([this](flecs::entity entity,
					 const ComponentStaticCollisionShapeName &collisionName) {
			auto transform = entity.get<ComponentStaticTransform>();
			if (transform && collisionName.shapeName != "") {
				OnStaticCollisionShape(entity, collisionName, *transform);
			}
		});

	flecs::query<ComponentBulletCollisionObject> queryCollisionObjects =
		ecs.query<ComponentBulletCollisionObject>();
}
