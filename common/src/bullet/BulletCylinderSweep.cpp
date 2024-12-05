#include "glm/ext/quaternion_geometric.hpp"

#include "../../../thirdparty/bullet/src/BulletCollision/BroadphaseCollision/btCollisionAlgorithm.h"
#include "../../../thirdparty/bullet/src/BulletCollision/CollisionShapes/btCylinderShape.h"
#include "../../../thirdparty/bullet/src/BulletCollision/CollisionDispatch/btCollisionWorld.h"
#include "../../../thirdparty/bullet/src/BulletCollision/CollisionDispatch/btCollisionObject.h"

#include "../../../ICon7/include/icon7/Debug.hpp"

#include "../../include/GlmBullet.hpp"
#include "../../include/CollisionWorld.hpp"
#include "../../include/EntityComponents.hpp"

#if not __unix__
#define M_PI 3.141592653589793
#else
#endif

bool CollisionWorld::TestCollisionMovementCylinder(
	ComponentShape shape, glm::vec3 start, glm::vec3 end,
	glm::vec3 *finalCorrectedPosition, bool *isOnGround, glm::vec3 *normal,
	float stepHeight, float minNormalYcomponent)
{
	LOG_TRACE("");
	if (normal) {
		*normal = glm::normalize(end - start);
	}
	bool wasOnGround = false;
	if (isOnGround) {
		wasOnGround = *isOnGround;
		*isOnGround = false;
	}

	const glm::vec3 movement = end - start;
	const glm::vec3 horizontalMovement = {movement.x, 0, movement.z};

	const float radius = shape.width / 2.0;
	const glm::vec3 safetyMarginXYZ(0.5, 0.5, 0.5);
	const glm::vec3 stepVector(0, stepHeight, 0);
	const glm::vec3 halfExtentXZ(radius, 0, radius);
	const glm::vec3 extentFromPos =
		halfExtentXZ + glm::vec3(0, shape.height, 0);

	const glm::vec3 aabbMin =
		glm::min(start, end) - halfExtentXZ - stepVector - safetyMarginXYZ;
	const glm::vec3 aabbMax =
		glm::max(start, end) + extentFromPos + stepVector + safetyMarginXYZ;

	std::vector<btCollisionObject *> objects;
	std::vector<Contact> contacts;
	GetObjectsInAABB(aabbMin, aabbMax, FILTER_TERRAIN, &objects);

	if (objects.size() == 0) {
		*finalCorrectedPosition = end;
		if (isOnGround) {
			*isOnGround = false;
		}
	LOG_TRACE("");
		return false;
	}

	LOG_TRACE("");
	const int stepsCount =
		glm::ceil(glm::min(glm::abs(movement.y) * 2.0f /
							   glm::clamp(shape.height, 0.01f, 1000000.0f),
						   glm::length(horizontalMovement) * 2.0f /
							   glm::clamp(shape.width, 0.01f, 1000000.0f))) +
		0.5f;

	btCylinderShape topCylinderShape(
		btVector3(radius, (shape.height - stepHeight) / 2.0, radius));
	btCollisionObject &collisionObject = *objectThatCanCollideWithTerrainAsCharacter;
	collisionObject.setCollisionShape(&topCylinderShape);
	const glm::vec3 topCylinderOffset =
		glm::vec3{0, (shape.height - stepHeight) / 2.0, 0};
	
	bool hasCollision = false;
	
	glm::vec3 badPosition = end;
	glm::vec3 goodPosition = start;
	
	glm::vec3 depenetrationVector{0,0,0};

	LOG_TRACE("");
	for (int i = 0; i <= stepsCount; ++i) {
		const glm::vec3 p = start + ((movement * float(i)) / float(stepsCount)) + depenetrationVector;
		collisionObject.setWorldTransform(btTransform(
			btQuaternion::getIdentity(), ToBullet(p + topCylinderOffset)));
		contacts.clear();
	LOG_TRACE("");
		if (TestObjectCollision(&collisionObject, objects, &contacts)) {
	LOG_TRACE("");
			for (Contact &c : contacts) {
				LOG_TRACE("Depth: %f,    distance: %f", c.depth, c.distance);
				if (c.depth > 0.02 && c.distance < -0.02) {
	LOG_TRACE("");
					hasCollision = true;
					if (normal) {
	LOG_TRACE("");
						if (glm::dot(c.normal, end - start) < glm::dot(*normal, end - start)) {
	LOG_TRACE("");
							*normal = c.normal;
							depenetrationVector = c.dir * c.distance;
						}
					}
				}
			}
			if (hasCollision) {
	LOG_TRACE("");
				badPosition = p;
				*finalCorrectedPosition = p + depenetrationVector;
				break;
			}
	LOG_TRACE("");
		}
	LOG_TRACE("");
		goodPosition = p;
	}
	
	LOG_TRACE("");
	btCylinderShape bottomCylinderShape(btVector3(radius, stepHeight * (wasOnGround ? 1.0f : 0.5f) , radius));
	collisionObject.setCollisionShape(&bottomCylinderShape);
	
	float maxHeight = finalCorrectedPosition->y - stepHeight;
	
	collisionObject.setWorldTransform(btTransform(
				btQuaternion::getIdentity(), ToBullet(*finalCorrectedPosition +
					glm::vec3{0, wasOnGround ? 0 : stepHeight*0.5f, 0}
					)));
	contacts.clear();
	bool onGround = false;
	if (TestObjectCollision(&collisionObject, objects, &contacts)) {
		for (Contact &c : contacts) {
	LOG_TRACE("");
			LOG_TRACE("Depth: %f,    distance: %f", c.depth, c.distance);
			if (c.depth > 0.02 && c.distance < -0.02) {
				hasCollision = true;
				if (normal) {
					if (glm::dot(c.normal, end - start) < glm::dot(*normal, end - start)) {
						*normal = c.normal;
					}
				}
			}
			if (c.normal.y > minNormalYcomponent) {
				onGround = true;
				if (maxHeight < c.point.y)
					maxHeight = c.point.y;
			}
		}
	}
	
	if (onGround) {
		if (isOnGround) {
			*isOnGround = true;
		}
		finalCorrectedPosition->y = maxHeight;
	}

	collisionObject.setCollisionShape(someCollisionShape);
	collisionObject.setWorldTransform(btTransform(
				btQuaternion::getIdentity(), {0, -1000000000.0f, 0}));
	LOG_TRACE("");
	return hasCollision;
}
