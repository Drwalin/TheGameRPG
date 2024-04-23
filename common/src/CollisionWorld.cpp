
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

class BroadphaseAabbAgregate : public btBroadphaseAabbCallback
{
public:
	int32_t filterGroup, filterMask;
	std::vector<btCollisionObject *> objects;
	BroadphaseAabbAgregate(int32_t filterGroup, int32_t filterMask)
		: filterGroup(filterGroup), filterMask(filterMask)
	{
	}
	virtual ~BroadphaseAabbAgregate() {}
	virtual bool process(const btBroadphaseProxy *proxy) override
	{
		if (proxy->m_collisionFilterGroup & filterGroup) {
			objects.emplace_back((btCollisionObject *)proxy->m_clientObject);
		}
		return true;
	}
};

bool CollisionWorld::TestCollisionMovement(
	EntityShape shape, glm::vec3 start, glm::vec3 end,
	glm::vec3 *finalCorrectedPosition, bool *isOnGround, glm::vec3 *normal,
	int approximationSpheresAmount, float stepHeight, float minNormalYcomponent,
	float maxDistancePerIteration) const
{
	approximationSpheresAmount = glm::max(approximationSpheresAmount, 3);
	approximationSpheresAmount = glm::min(approximationSpheresAmount, 10);
	const float radius = shape.width / 2.0;
	const glm::vec3 safetyMarginXYZ(2, 2, 2);
	const glm::vec3 stepVector(0, stepHeight, 0);
	const glm::vec3 halfExtentXZ(radius, 0, radius);
	const glm::vec3 extentFromPos =
		halfExtentXZ + glm::vec3(0, shape.height, 0);
	const glm::vec3 aabbMin =
		glm::min(start, end) - halfExtentXZ - stepVector - safetyMarginXYZ;
	const glm::vec3 aabbMax =
		glm::max(start, end) + extentFromPos + stepVector + safetyMarginXYZ;

	BroadphaseAabbAgregate broadphaseCallback(FILTER_GROUP_TERRAIN,
											  FILTER_MASK_TERRAIN);

	collisionWorld->getBroadphase()->aabbTest(
		ToBullet(aabbMin), ToBullet(aabbMax), broadphaseCallback);

	if (broadphaseCallback.objects.size() == 0) {
		*finalCorrectedPosition = end;
		if (isOnGround) {
			glm::vec3 p, n;
			*isOnGround =
				TestIsOnGround(end, &p, &n, stepHeight, minNormalYcomponent);
			if (*isOnGround) {
				*finalCorrectedPosition = p;
			}
		}
		return false;
	}

	const float heightStart = stepHeight + radius;
	const float heightEnd = shape.height - radius;
	const float heightDiff = heightEnd - heightStart;
	const float heightSpheresDistance =
		heightDiff / (approximationSpheresAmount - 2);

	btSphereShape _shape(radius);
	btCollisionObject sphere;
	sphere.setCollisionShape(&_shape);

	const auto &objects = broadphaseCallback.objects;

	std::vector<std::vector<Contact>> contacts;
	contacts.resize(approximationSpheresAmount);

	// TODO: first push out of terrain

	// perform spheres-trace cast

	glm::vec3 totalMovement = end - start;
	float totalMovementLength = glm::length(totalMovement);
	glm::vec3 direction = totalMovement / totalMovementLength;

	float maxDistanceToTravel = totalMovementLength;
	
	printf("\n");
	LOG_TRACE("height:      %f", shape.height);
	LOG_TRACE("width:       %f", shape.width);
	LOG_TRACE("radius:      %f", radius);
	LOG_TRACE("step height: %f", stepHeight);

	for (int i = 1; i < approximationSpheresAmount; ++i) {
		std::vector<Contact> *contactsForThis = &(contacts[i]);

		const float currentHeight = heightStart + heightSpheresDistance * (i-1);

		glm::vec3 point = start + glm::vec3(0, currentHeight, 0);
		
		LOG_TRACE("Testing upper body sphere bottom at height: %f", point.y-radius);

		const int steps =
			glm::ceil(maxDistanceToTravel / maxDistancePerIteration);
		const float step = maxDistanceToTravel / steps - 0.000001f;
		float distanceTraveled = 0;
		if (PerformObjectSweep(&sphere, point, direction, step,
							   maxDistanceToTravel, objects, contactsForThis,
							   &distanceTraveled)) {
			FindCorrectTravelDistance(*contactsForThis, start,
									  distanceTraveled, &maxDistanceToTravel);
		}
	}

	glm::vec3 finalPosition = start + direction * maxDistanceToTravel;
	*finalCorrectedPosition = finalPosition;
	
	{
		glm::vec3 a = start;
		glm::vec3 b = finalPosition;
		LOG_TRACE("Test upper body: (%f %f %f) -> (%f %f %f)", a.x, a.y, a.z, b.x, b.y, b.z);
	}
	
	// test feet's sphere
	std::vector<Contact> *contactsForThis = &(contacts[0]);
	
	const float sweepBegHeight = radius + stepHeight;
	maxDistanceToTravel = stepHeight * 2;
	
	const glm::vec3 sweepBeg = finalPosition + glm::vec3(0, sweepBegHeight, 0);
	
	{
		glm::vec3 a = sweepBeg - glm::vec3(0,radius,0);
		glm::vec3 b = sweepBeg + glm::vec3(0,-1,0) * maxDistanceToTravel - glm::vec3(0,radius,0);
		LOG_TRACE("Testing feet: (%f %f %f) -> (%f %f %f)", a.x, a.y, a.z, b.x, b.y, b.z);
	}
	
	const int steps =
		glm::ceil(maxDistanceToTravel / maxDistancePerIteration);
	const float step = maxDistanceToTravel / steps - 0.000001f;
	float distanceTraveled = 0, dt1 = 0;
	if (PerformObjectSweep(&sphere, sweepBeg, glm::vec3(0,-1,0), step,
						   maxDistanceToTravel, objects, contactsForThis,
						   &distanceTraveled)) {
		dt1 = distanceTraveled;
		FindCorrectTravelDistance(*contactsForThis, start,
								  distanceTraveled, &distanceTraveled);
		if (distanceTraveled < stepHeight) {
			*finalCorrectedPosition = sweepBeg + glm::vec3(0,-1,0) * distanceTraveled - glm::vec3(0,radius,0);
		} else {
			// can go down
		}
	}
	
	{
		glm::vec3 a = sweepBeg - glm::vec3(0,radius,0);
		glm::vec3 b = sweepBeg + glm::vec3(0,-1,0) * distanceTraveled - glm::vec3(0,radius,0);
		LOG_TRACE("Test feet: (%f %f %f) ==( %f >> %f / %f )=> (%f %f %f)", a.x, a.y, a.z, dt1, distanceTraveled, maxDistanceToTravel, b.x, b.y, b.z);
	}
	
	// TODO: test isGround
	if (isOnGround) {
		bool wasOnGround = *isOnGround;
		*isOnGround = false;
		glm::vec3 p, n;
		if (TestIsOnGround(finalPosition, &p, &n, stepHeight, minNormalYcomponent)) {
			glm::vec3 p2 = sweepBeg + glm::vec3(0,-1,0) * distanceTraveled - glm::vec3(0,radius,0);
			
			{
				glm::vec3 a = p;
				glm::vec3 b = p2;
				LOG_TRACE("p (%f %f %f) p2 (%f %f %f)", a.x, a.y, a.z, b.x, b.y, b.z);
			}
			
			bool doCorrection = false;
			
			if (p.y >= p2.y && distanceTraveled < stepHeight) {
				doCorrection = true;
				*isOnGround = true;
			} else {
				doCorrection = wasOnGround;
			}
			
			if (doCorrection) {
				if (p2.y > p.y) {
					*finalCorrectedPosition = p2;
				} else {
					*finalCorrectedPosition = p;
				}
			}
		}
		LOG_TRACE("Distance traveled = %f", distanceTraveled);
		{
			glm::vec3 a = finalPosition;
			glm::vec3 b = *finalCorrectedPosition;
			float h1 = finalPosition.y-stepHeight, h2=finalPosition.y+stepHeight;
			LOG_TRACE("Test onGround: (%f %f %f) -> (%f %f %f)   (%f ... %f)   %s", a.x, a.y, a.z, b.x, b.y, b.z, h1, h2, *isOnGround ? "ON GROUND" : "FALLING");
		}
	}
	
	{
		glm::vec3 a = start;
		glm::vec3 b = *finalCorrectedPosition;
		LOG_TRACE("Total movement: (%f %f %f) -> (%f %f %f)", a.x, a.y, a.z, b.x, b.y, b.z);
	}

	// TODO: solve somewhere FindPushoutVector

	
	
	if (normal) {
		for (int i=contacts.size(); i>0; --i) {
			auto &c = contacts[(i)%contacts.size()];
			if (c.size()) {
				*normal = c.back().normal;
				break;
			}
		}
	}
	
	
	
	
	
	
	return false;
}

void CollisionWorld::FindCorrectTravelDistance(
	const std::vector<Contact> &contacts, glm::vec3 start,
	float currentTraveledDistance, float *correctedTravelDistance) const
{
	float minTravelDistance = currentTraveledDistance;
	float maxTravelDistance = currentTraveledDistance;
	for (const Contact &c : contacts) {
		float dot = glm::dot(c.dir, c.normal);
		if (dot < -0.1) { // push backward
			maxTravelDistance =
				glm::min(maxTravelDistance, c.distance - c.depth);
		} else if (dot > 0.1) { // push forward
			minTravelDistance =
				glm::max(minTravelDistance, c.distance + c.depth);
		} else { // push normal
				 // IGNORE this here and solve in FindPushoutVector()
		}
	}

	maxTravelDistance = glm::max(maxTravelDistance, 0.0f);

	*correctedTravelDistance = (maxTravelDistance + minTravelDistance) * 0.5f;
}

void CollisionWorld::FindPushoutVector(const std::vector<Contact> &contacts,
									   glm::vec3 position,
									   glm::vec3 *pushoutVector) const
{
	LOG_FATAL("Not implemented yet");
}

bool CollisionWorld::PerformObjectSweep(
	btCollisionObject *object, glm::vec3 start, glm::vec3 dir, float step,
	float maxDistance, const std::vector<btCollisionObject *> &otherObjects,
	std::vector<Contact> *contacts, float *distanceTraveled) const
{
	bool hasCollision = false;
	*distanceTraveled = 0;
	while (*distanceTraveled < maxDistance) {
		int contactsOffset = contacts->size();
		glm::vec3 point = start + dir * *distanceTraveled;
		object->setWorldTransform(btTransform(btQuaternion(), ToBullet(point)));
		if (TestObjectCollision(object, otherObjects, contacts)) {
			for (int i = contactsOffset; i < contacts->size(); ++i) {
				Contact &c = contacts->at(i);
				c.distance = *distanceTraveled;
				c.dir = dir;
				c.objectPos = point;
			}
			for (int i = contactsOffset; i < contacts->size(); ++i) {
				hasCollision = true;
				Contact &c = contacts->at(i);
				if (glm::dot(c.dir, c.normal) < 0.0f) {
					return true;
				}
			}
		}

		if (*distanceTraveled + step > maxDistance) {
			if (*distanceTraveled > maxDistance) {
				return hasCollision;
			}
			if (maxDistance - *distanceTraveled < 0.001) {
				return hasCollision;
			} else {
				*distanceTraveled = maxDistance;
			}
		} else {
			*distanceTraveled += step;
		}
	}
	return hasCollision;
}

bool CollisionWorld::TestObjectCollision(
	btCollisionObject *object,
	const std::vector<btCollisionObject *> &otherObjects,
	std::vector<Contact> *contacts) const
{
	bool hasResult = false;
	btCollisionObjectWrapper sphereWrapper(nullptr, object->getCollisionShape(),
										   object, object->getWorldTransform(),
										   -1, -1);
	for (btCollisionObject *o : otherObjects) {
		btCollisionObjectWrapper otherWrapper(
			nullptr, o->getCollisionShape(), o, o->getWorldTransform(), -1, -1);
		btCollisionAlgorithm *algo =
			collisionWorld->getDispatcher()->findAlgorithm(
				&sphereWrapper, &otherWrapper, 0, BT_CLOSEST_POINT_ALGORITHMS);
		if (algo) {
			class ManifoldResult : public btManifoldResult
			{
			public:
				std::vector<Contact> *contacts;
				btCollisionObject *object;
				bool has = false;
				ManifoldResult(const btCollisionObjectWrapper *body0Wrap,
							   const btCollisionObjectWrapper *body1Wrap,
							   std::vector<Contact> *contacts)
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
					if (d.length2() > radius*radius) {
						return;
					}
					
					LOG_DEBUG("depth:%f    origin:%f     hitpoint:%f", depth, origin.y(), pointInWorld.y());
					
					contacts->push_back({ToGlm(normalOnBInWorld),
										 ToGlm(pointInWorld),
										 fabs(depth),
										 {0, 0, 0},
										 0,
										 {0,0,0}});
					has = true;
				}
			} result(&sphereWrapper, &otherWrapper, contacts);
			result.object = object;
			algo->processCollision(&sphereWrapper, &otherWrapper,
								   collisionWorld->getDispatchInfo(), &result);
			collisionWorld->getDispatcher()->freeCollisionAlgorithm(algo);
			hasResult |= result.has;
		}
	}
	return hasResult;
}

bool CollisionWorld::TestIsOnGround(glm::vec3 pos, glm::vec3 *groundPoint,
									glm::vec3 *normal, float stepHeight,
									float minNormalYcomponent) const
{
	glm::vec3 hp, _normal;
	if (RayTestFirstHitTerrain(pos + glm::vec3{0, stepHeight, 0},
							   pos - glm::vec3{0, stepHeight, 0}, &hp,
							   &_normal, nullptr)) {
		if (fabs(_normal.y) >= minNormalYcomponent) {
			if (normal)
				*normal = _normal;
			if (groundPoint) {
				*groundPoint = hp;
			}
			return true;
		}
	}
	if (normal)
		*normal = {0,0,0};
	if (groundPoint)
		*groundPoint = {0,0,0};
	return false;
}

class ClosestRayResultNotMe : public btCollisionWorld::ClosestRayResultCallback
{
public:
	ClosestRayResultNotMe(btCollisionObject *me, glm::vec3 start, glm::vec3 end)
		: btCollisionWorld::ClosestRayResultCallback(ToBullet(start), ToBullet(end))
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
	ClosestRayResultNotMe cb(object, start, end);
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
	btCollisionWorld::ClosestRayResultCallback cb(ToBullet(start), ToBullet(end));
	cb.m_collisionFilterMask = FILTER_GROUP_TERRAIN;
	collisionWorld->rayTest(ToBullet(start), ToBullet(end), cb);
	if (cb.hasHit() == false) {
		if (travelFactor)
			*travelFactor = 1;
		if (hitNormal)
			*hitNormal = {0,0,0};
		if (hitPosition)
			*hitPosition = end;
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
