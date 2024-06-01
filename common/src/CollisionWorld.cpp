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
	while (entities.empty() == false) {
		DeleteEntity(entities.begin()->first);
	}
	while (collisionWorld->getNumCollisionObjects() != 0) {
		btCollisionObject *object =
			collisionWorld->getCollisionObjectArray()[0];
		RemoveAndDestroyCollisionObject(object);
	}
}

void CollisionWorld::RemoveAndDestroyCollisionObject(btCollisionObject *object)
{
	collisionWorld->removeCollisionObject(object);

	delete object;

	btCollisionShape *shape = object->getCollisionShape();
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

void CollisionWorld::LoadStaticCollision(const TerrainCollisionData *data)
{
	btTriangleMesh *triangles =
		new btTriangleMesh((data->vertices.size() / 3) > (1 << 15), false);
	triangles->preallocateVertices(data->vertices.size());
	triangles->preallocateIndices(data->indices.size());
	std::vector<uint32_t> map;
	map.resize(data->vertices.size());
	for (int i = 0; i < data->vertices.size(); ++i) {
		const glm::vec3 &v = data->vertices[i];
		map[i] = triangles->findOrAddVertex({v.x, v.y, v.z}, false);
	}
	for (int i = 0; i + 2 < data->indices.size(); i += 3) {
		const uint32_t *idx = &(data->indices[i]);
		triangles->addTriangleIndices(idx[0], idx[1], idx[2]);
	}
	btBvhTriangleMeshShape *shape =
		new btBvhTriangleMeshShape(triangles, false, true);
	shape->buildOptimizedBvh();
	btCollisionObject *object = AllocateNewCollisionObject();
	object->setCollisionShape(shape);
	object->setUserIndex(FILTER_TERRAIN);
	collisionWorld->addCollisionObject(object, btBroadphaseProxy::StaticFilter);
	collisionWorld->updateSingleAabb(object);
}

bool CollisionWorld::AddEntity(uint64_t entityId, EntityShape shape,
							   glm::vec3 pos)
{
	auto it = entities.find(entityId);
	if (it != entities.end()) {
		LOG_WARN("Error: entity with ID=%lu already exists in CollisionWorld.",
				 entityId);
		UpdateEntityBvh(entityId, shape, pos);
		return false;
	}
	btCapsuleShape *_shape =
		new btCapsuleShape(shape.width * 0.5, shape.height);
	btCollisionObject *object = AllocateNewCollisionObject();
	object->setCollisionShape(_shape);
	entities[entityId] = object;
	object->setWorldTransform(btTransform(btQuaternion(), ToBullet(pos)));
	object->setUserIndex(FILTER_ENTITY);
	object->setUserIndex2(((uint32_t)(entityId)) & 0xFFFFFFFF);
	object->setUserIndex3(((uint32_t)(entityId >> 32)) & 0xFFFFFFFF);
	collisionWorld->addCollisionObject(object,
									   btBroadphaseProxy::CharacterFilter);
	collisionWorld->updateSingleAabb(object);
	return true;
}

void CollisionWorld::UpdateEntityBvh(uint64_t entityId, EntityShape shape,
									 glm::vec3 pos)
{
	auto it = entities.find(entityId);
	if (it == entities.end()) {
		return;
	}
	it->second->setWorldTransform(btTransform(btQuaternion(), ToBullet(pos)));
	collisionWorld->updateSingleAabb(it->second);
}

void CollisionWorld::DeleteEntity(uint64_t entityId)
{
	auto it = entities.find(entityId);
	if (it == entities.end()) {
		return;
	}
	RemoveAndDestroyCollisionObject(it->second);
	entities.erase(it);
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
	ecs.observer<EntityShape, EntityMovementState>()
		.event(flecs::OnAdd)
		.each([this](flecs::entity entity, const EntityShape &shape,
					 const EntityMovementState &state) {
			this->AddEntity(entity.id(), shape, state.pos);
		});
	ecs.observer<EntityShape, EntityMovementState>()
		.event(flecs::OnRemove)
		.each([this](flecs::entity entity, const EntityShape &shape,
					 const EntityMovementState &state) {
			this->DeleteEntity(entity.id());
		});
	ecs.observer<EntityShape>()
		.event(flecs::OnSet)
		.each([this](flecs::entity entity, const EntityShape &shape) {
			const EntityMovementState *state =
				entity.get<EntityMovementState>();
			if (entity.has<EntityMovementState>()) {
				this->UpdateEntityBvh(entity.id(), shape, state->pos);
			}
		});
}
