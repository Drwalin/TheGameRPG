
#include <icon7/Debug.hpp>

#include <bullet/LinearMath/btVector3.h>
#include <bullet/btBulletCollisionCommon.h>
#include <bullet/BulletDynamics/Character/btKinematicCharacterController.h>

#include "../include/GlmBullet.hpp"

#include "../include/Realm.hpp"

#include "../include/CollisionWorld.hpp"

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

CollisionWorld::CollisionWorld(Realm *realm)
{
	this->realm = realm;
	broadphase = new btDbvtBroadphase();
	collisionConfiguration = new btDefaultCollisionConfiguration();
	dispatcher = new btCollisionDispatcher(collisionConfiguration);
	collisionWorld =
		new btCollisionWorld(dispatcher, broadphase, collisionConfiguration);
	updateWorldBvh = true;
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
	collisionWorld->addCollisionObject(object, FILTER_GROUP_TERRAIN,
									   FILTER_MASK_TERRAIN);
}

bool CollisionWorld::AddEntity(uint64_t entityId, EntityShape shape,
							   glm::vec3 pos)
{
	auto it = entities.find(entityId);
	if (it != entities.end()) {
		LOG_DEBUG("Error: entity with ID=%lu already exists in CollisionWorld.",
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
	object->setUserIndex2(((uint32_t)(entityId)) & 0xFFFFFFFF);
	object->setUserIndex3(((uint32_t)(entityId >> 32)) & 0xFFFFFFFF);
	collisionWorld->addCollisionObject(object, FILTER_GROUP_ENTITY,
									   FILTER_MASK_ENTITY);
	updateWorldBvh = true;
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
	updateWorldBvh = true;
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

bool CollisionWorld::TestCollisionMovementRays(
	EntityShape shape, glm::vec3 start, glm::vec3 end,
	glm::vec3 *finalCorrectedPosition, bool *isOnGround,
	glm::vec3 *normal) const
{
	LOG_FATAL("Not implemented");
	bool onGround = false;
	const glm::vec3 halfHeight(0, shape.height*0.5, 0);
	glm::vec3 movement = end - start;
	// perform multi-ray cast
	
	glm::vec3 hp, _normal;
	const glm::vec3 startFeet = start - halfHeight;
	const glm::vec3 startMid = start;
	const glm::vec3 startHead = start + halfHeight;
	glm::vec3 endFeet = startFeet + movement;
	glm::vec3 endMid = startMid + movement;
	glm::vec3 endHead = startHead + movement;
	
	// ray test feet
	if (RayTestFirstHitTerrain(startFeet, endFeet, &hp, &_normal, nullptr)) {
		if (_normal.y > 0.7) {
			*isOnGround = true;
		}
	}
	
	
	// ray test mid-body
	// ray test head
	
	// ray test mid-body to feet
	
	// if no collision, ray test feet
	
	
	if (isOnGround) {
		*isOnGround = onGround;
	}
}

bool CollisionWorld::TestCollisionMovementMultiStep(EntityShape shape, glm::vec3 _start,
							   glm::vec3 _end, glm::vec3 *finalCorrectedPosition,
							   bool *isOnGround, glm::vec3 *normal, float stepSize) const
{
	const glm::vec3 diff = _end-_start;
	const float diffLength = diff.length();
	int steps = (diffLength+stepSize+0.001f)/stepSize;
	glm::vec3 step = diff*(1.0f/steps);
	glm::vec3 start, end = _start;
	for (int i=0; i<steps; ++i) {
		start = end;
		end = start + step;
		if (TestCollisionMovement(shape, start, end, finalCorrectedPosition, isOnGround, normal)) {
			return true;
		}
	}
	return false;
}

bool CollisionWorld::TestCollisionMovement(EntityShape shape, glm::vec3 start,
										   glm::vec3 end,
										   glm::vec3 *finalCorrectedPosition,
										   bool *isOnGround,
										   glm::vec3 *normal) const
{
	
	glm::vec3 center = glm::vec3{0, shape.height * 0.5, 0};
	start += center;
	end += center;
	const glm::vec3 dm = (end - start);
	btCapsuleShape _shape(shape.width * 0.5, shape.height - shape.width);
	if (isOnGround) {
		*isOnGround = false;
	}
	btCollisionWorld::ClosestConvexResultCallback cb(ToBullet(start),
													 ToBullet(end));
	cb.m_collisionFilterGroup = FILTER_GROUP_TERRAIN;
	cb.m_collisionFilterMask = FILTER_GROUP_TERRAIN;
	// perform convex sweep test
	collisionWorld->convexSweepTest(
		&_shape, btTransform{btQuaternion{}, ToBullet(start)},
		btTransform{btQuaternion{}, ToBullet(end)}, cb, -0.01);

	if (cb.hasHit()) {
		glm::vec3 n = ToGlm(cb.m_hitNormalWorld);
		if (glm::dot(n, dm) < 0) {
			if (normal)
				*normal = n;

			*finalCorrectedPosition = start + dm * cb.m_closestHitFraction - center;

			if (isOnGround) {
				glm::vec3 hp, _normal;
				if (RayTestFirstHitTerrain(
						*finalCorrectedPosition + glm::vec3{0.0f, 0.07f, 0.0f},
						*finalCorrectedPosition - glm::vec3{0.0f, 0.13f, 0.0f}, &hp,
						&_normal, nullptr)) {
					if (fabs(_normal.y) > 0.7) {
						*isOnGround = true;
					}
				}
			}
			return true;
		}
	}
	
	*finalCorrectedPosition = end - center;
	return false;
}

class ClosestRayResultNotMe : public btCollisionWorld::ClosestRayResultCallback
{
public:
	ClosestRayResultNotMe(btCollisionObject *me)
		: btCollisionWorld::ClosestRayResultCallback(btVector3(0.0, 0.0, 0.0),
													 btVector3(0.0, 0.0, 0.0))
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

bool CollisionWorld::RayTestFirstHit(glm::vec3 start, glm::vec3 end,
									 glm::vec3 *hitPosition,
									 glm::vec3 *hitNormal, uint64_t *entityId,
									 float *travelFactor, bool *hasNormal,
									 uint64_t ignoreEntityId) const
{
	btCollisionObject *object = nullptr;
	auto it = entities.find(ignoreEntityId);
	if (it != entities.end()) {
		object = it->second;
	}
	ClosestRayResultNotMe cb(object);
	cb.m_collisionFilterMask = FILTER_MASK_ALL;
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

bool CollisionWorld::RayTestFirstHitTerrain(glm::vec3 start, glm::vec3 end,
											glm::vec3 *hitPosition,
											glm::vec3 *hitNormal,
											float *travelFactor) const
{
	btCollisionWorld::ClosestRayResultCallback cb({}, {});
	cb.m_collisionFilterMask = FILTER_GROUP_TERRAIN;
	collisionWorld->rayTest(ToBullet(start), ToBullet(end), cb);
	if (cb.hasHit() == false) {
		if (travelFactor)
			*travelFactor = 1;
		return false;
	}

	if (hitPosition)
		*hitPosition = ToGlm(cb.m_hitPointWorld);
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

void CollisionWorld::RegisterSystems(Realm *realm) { return; }
