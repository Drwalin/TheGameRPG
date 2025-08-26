#include "../../../thirdparty/flecs/include/flecs.h"

#include "../../../ICon7/include/icon7/Debug.hpp"

#include "../../include/CollisionWorld.hpp"
#include "../../include/EntityComponents.hpp"

bool CollisionWorld::TestCollisionMovement(ComponentShape shape, glm::vec3 start,
						   glm::vec3 end, glm::vec3 *finalCorrectedPosition,
						   bool *isOnGround, glm::vec3 *normal,
						   float stepHeight,
						   float minNormalYcomponent) const
{
	if (normal) {
		*normal = glm::normalize(end - start);
	}
	
	bool wasOnGround = false;
	if (isOnGround) {
		wasOnGround = *isOnGround;
		*isOnGround = false;
	}
	
	std::vector<flecs::entity> &entities = *(std::vector<flecs::entity>*)&localEntitiesStack;
	entities.clear();
	
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

	size_t count = TestForEntitiesAABBApproximate(aabbMin, aabbMax, &entities, FILTER_TERRAIN | FILTER_STATIC_OBJECT);
	
	if (count == 0) {
		*finalCorrectedPosition = end;
		if (isOnGround) {
			*isOnGround = false;
		}
		return false;
	}

	const float heightStart = stepHeight;
	const float heightEnd = shape.height - 0.001f;
	const float heightDiff = heightEnd - heightStart;
	Collision3D::Cylinder cyl{shape.height, shape.width*0.5f};

	const glm::vec3 travelDirNorm = glm::normalize(end - start);

	bool hasCollision = false;
	
	float maxValidTravelDistanceFactor = 1.0f;
	
	Collision3D::RayInfo movement;
	movement.Calc(start, end);
	
	for (int i=0; i<count; ++i) {
		flecs::entity e = entities[i];
		if (auto *t = e.try_get<ComponentStaticTransform>()) {
			auto *s = e.try_get<ComponentCollisionShape>();
			if (s == nullptr) {
				LOG_FATAL("Static entity %lu does not have ComponentCollisionShape but is inside CollisionWorld",
						  e.id());
				continue;
			}
			
			float travelDistanceFactor = 1.0;
			glm::vec3 _normal;
			if (s->shape.CylinderTestMovement(t->trans, travelDistanceFactor, cyl, movement, _normal)) {
				if (maxValidTravelDistanceFactor >= travelDistanceFactor) {
					maxValidTravelDistanceFactor = travelDistanceFactor;
					if (normal) *normal = _normal;
				}
			}
		} else {
			LOG_FATAL("Static entity %lu does not have ComponentStaticTransform nor but is inside CollisionWorld",
					  e.id());
		}
	}
	
	*finalCorrectedPosition = start + (end - start) * maxValidTravelDistanceFactor;
	
	float maxOffsetHeight = 0;
	
	Collision3D::RayInfo toGroundRay;
	toGroundRay.Calc(*finalCorrectedPosition, *finalCorrectedPosition - stepHeight);
	
	for (int i=0; i<count; ++i) {
		flecs::entity e = entities[i];
		if (auto *t = e.try_get<ComponentStaticTransform>()) {
			auto *s = e.try_get<ComponentCollisionShape>();
			if (s == nullptr) {
				LOG_FATAL("Static entity %lu does not have ComponentCollisionShape but is inside CollisionWorld",
						  e.id());
				continue;
			}
			
			float near;
			glm::vec3 n;
			if (s->shape.RayTest(t->trans, toGroundRay, near, n)) {
				
			}
		} else {
			LOG_FATAL("Static entity %lu does not have ComponentStaticTransform nor but is inside CollisionWorld",
					  e.id());
		}
	}
	
	
	
	
	
	

	*finalCorrectedPosition = end;

	// TODO: test isOnGround
	const glm::vec3 toHeadTop = glm::vec3(0, heightEnd, 0);
	const glm::vec3 toMaxFeet = glm::vec3(0, stepHeight, 0);
	const glm::vec3 toFeetBottom =
		glm::vec3(0, -0.01, 0) +
		((wasOnGround) ? -glm::vec3(0, stepHeight, 0) : glm::vec3(0, 0, 0));
	const glm::vec3 toMid = toHeadTop * 0.5f;
	const glm::vec3 toFeetStart = toMid + glm::vec3(0, 0.1, 0);
	// const glm::vec3 toHeadStart = toMid - glm::vec3(0, 0.1, 0);

	glm::vec3 hitPoint, hitNormal;
	float travelFactor;

	// test for feet collision and onGround
	if (RayTestFirstHitWithObjects(end + toFeetStart,
								   toFeetBottom - toFeetStart, &hitPoint,
								   &hitNormal, &travelFactor, objects)) {
		hasCollision = true;
		if (hitPoint.y > toMaxFeet.y + end.y) {
			// TODO: floor collision is too high, need to retrace backward, to
			//       stop on vertical obstacle
		}
		*finalCorrectedPosition = end = hitPoint;
		if (isOnGround) {
			*isOnGround = (hitNormal.y >= minNormalYcomponent);
		}
		if (normal) {
			if (glm::dot(hitNormal, end - start) <
				glm::dot(*normal, end - start)) {
				*normal = hitNormal;
			}
		}
	}

	return hasCollision;
}
