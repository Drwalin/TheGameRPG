#include <icon7/Debug.hpp>

#include "../../../thirdparty/bullet/src/BulletCollision/BroadphaseCollision/btCollisionAlgorithm.h"

#include "../../include/GlmBullet.hpp"

#include "../../include/EntityComponents.hpp"
#include "BulletPhysicsCallbacks.hpp"
#include "../../include/CollisionWorld.hpp"

bool CollisionWorld::TestCollisionMovement(
	ComponentShape shape, glm::vec3 start, glm::vec3 end,
	glm::vec3 *finalCorrectedPosition, bool *isOnGround, glm::vec3 *normal,
	int approximationSpheresAmount, float stepHeight, float minNormalYcomponent,
	float maxDistancePerIteration) const
{
	LOG_FATAL("Test running");
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

	std::vector<btCollisionObject *> objects;
	GetObjectsInAABB(aabbMin, aabbMax, FILTER_TERRAIN, &objects);

	if (objects.size() == 0) {
		*finalCorrectedPosition = end;
		if (isOnGround) {
			*isOnGround = false;
		}

		return false;
	}

	printf("\n");
	LOG_TRACE("height:      %f", shape.height);
	LOG_TRACE("width:       %f", shape.width);
	LOG_TRACE("radius:      %f", radius);
	LOG_TRACE("step height: %f", stepHeight);

	const float heightStart = stepHeight + radius;
	const float heightEnd = shape.height - radius;
	const float heightDiff = heightEnd - heightStart;
	const float heightSpheresDistance =
		heightDiff / (approximationSpheresAmount - 2);

	btSphereShape _shape(radius);
	btCollisionObject sphere;
	sphere.setCollisionShape(&_shape);

	std::vector<std::vector<Contact>> contacts;
	contacts.resize(approximationSpheresAmount);

	// TODO: first push out of terrain

	// perform spheres-trace cast

	glm::vec3 totalMovement = end - start;
	float totalMovementLength = glm::length(totalMovement);
	glm::vec3 direction = totalMovement / totalMovementLength;

	float maxDistanceToTravel = totalMovementLength;

	for (int i = 1; i < approximationSpheresAmount; ++i) {
		std::vector<Contact> *contactsForThis = &(contacts[i]);

		const float currentHeight =
			heightStart + heightSpheresDistance * (i - 1);

		glm::vec3 point = start + glm::vec3(0, currentHeight, 0);

		LOG_TRACE("Testing upper body sphere bottom at height: %f",
				  point.y - radius);

		const int steps =
			glm::ceil(maxDistanceToTravel / maxDistancePerIteration);
		const float step = maxDistanceToTravel / steps - 0.000001f;
		float distanceTraveled = 0;
		if (PerformObjectSweep(&sphere, point, direction, step,
							   maxDistanceToTravel, objects, contactsForThis,
							   &distanceTraveled)) {
			FindCorrectTravelDistance(*contactsForThis, start, distanceTraveled,
									  &maxDistanceToTravel);
		}
	}

	glm::vec3 finalPosition = start + direction * maxDistanceToTravel;
	*finalCorrectedPosition = finalPosition;

	{
		glm::vec3 a = start;
		glm::vec3 b = finalPosition;
		LOG_TRACE("Test upper body: (%f %f %f) -> (%f %f %f)", a.x, a.y, a.z,
				  b.x, b.y, b.z);
	}

	// test feet's sphere
	std::vector<Contact> *contactsForThis = &(contacts[0]);

	const float sweepBegHeight = radius + stepHeight;
	maxDistanceToTravel = stepHeight * 2;

	const glm::vec3 sweepBeg = finalPosition + glm::vec3(0, sweepBegHeight, 0);

	{
		glm::vec3 a = sweepBeg - glm::vec3(0, radius, 0);
		glm::vec3 b = sweepBeg + glm::vec3(0, -1, 0) * maxDistanceToTravel -
					  glm::vec3(0, radius, 0);
		LOG_TRACE("Testing feet: (%f %f %f) -> (%f %f %f)", a.x, a.y, a.z, b.x,
				  b.y, b.z);
	}

	const int steps = glm::ceil(maxDistanceToTravel / maxDistancePerIteration);
	const float step = maxDistanceToTravel / steps - 0.000001f;
	float distanceTraveled = 0;
	float dt1 = 0;
	if (PerformObjectSweep(&sphere, sweepBeg, glm::vec3(0, -1, 0), step,
						   maxDistanceToTravel, objects, contactsForThis,
						   &distanceTraveled)) {
		dt1 = distanceTraveled;
		FindCorrectTravelDistance(*contactsForThis, start, distanceTraveled,
								  &distanceTraveled);
		if (distanceTraveled < stepHeight) {
			*finalCorrectedPosition = sweepBeg +
									  glm::vec3(0, -1, 0) * distanceTraveled -
									  glm::vec3(0, radius, 0);
		} else {
			// can go down
		}
	}

	{
		glm::vec3 a = sweepBeg - glm::vec3(0, radius, 0);
		glm::vec3 b = sweepBeg + glm::vec3(0, -1, 0) * distanceTraveled -
					  glm::vec3(0, radius, 0);
		LOG_TRACE("Test feet: (%f %f %f) ==( %f >> %f / %f )=> (%f %f %f)", a.x,
				  a.y, a.z, dt1, distanceTraveled, maxDistanceToTravel, b.x,
				  b.y, b.z);
	}

	// TODO: test isGround
	if (isOnGround) {
		bool wasOnGround = *isOnGround;
		*isOnGround = false;
		glm::vec3 p, n;
		if (TestIsOnGround(finalPosition, &p, &n, stepHeight,
						   minNormalYcomponent)) {
			glm::vec3 p2 = sweepBeg + glm::vec3(0, -1, 0) * distanceTraveled -
						   glm::vec3(0, radius, 0);

			{
				glm::vec3 a = p;
				glm::vec3 b = p2;
				LOG_TRACE("p (%f %f %f) p2 (%f %f %f)", a.x, a.y, a.z, b.x, b.y,
						  b.z);
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
			float h1 = finalPosition.y - stepHeight,
				  h2 = finalPosition.y + stepHeight;
			LOG_TRACE(
				"Test onGround: (%f %f %f) -> (%f %f %f)   (%f ... %f)   %s",
				a.x, a.y, a.z, b.x, b.y, b.z, h1, h2,
				*isOnGround ? "ON GROUND" : "FALLING");
		}
	}

	{
		glm::vec3 a = start;
		glm::vec3 b = *finalCorrectedPosition;
		LOG_TRACE("Total movement: (%f %f %f) -> (%f %f %f)", a.x, a.y, a.z,
				  b.x, b.y, b.z);
	}

	// TODO: solve somewhere FindPushoutVector

	// get normal
	if (normal) {
		for (int i = contacts.size(); i > 0; --i) {
			auto &c = contacts[(i) % contacts.size()];
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
	while (true) {
		int contactsOffset = contacts->size();
		glm::vec3 point = start + dir * *distanceTraveled;
		object->setWorldTransform(
			btTransform(btQuaternion::getIdentity(), ToBullet(point)));
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
					LOG_DEBUG("sweep ends at distance: %f", *distanceTraveled);
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
			ManifoldResult result(&sphereWrapper, &otherWrapper, contacts);
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
							   pos - glm::vec3{0, stepHeight, 0}, &hp, &_normal,
							   nullptr)) {
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
		*normal = {0, 0, 0};
	if (groundPoint)
		*groundPoint = {0, 0, 0};
	return false;
}
