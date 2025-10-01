#include "../../../thirdparty/flecs/distr/flecs.h"

#include "../../../ICon7/include/icon7/Debug.hpp"

#include "../../include/CollisionWorld.hpp"
#include "../../include/EntityComponents.hpp"

bool CollisionWorld_spp::TestCollisionMovement(ComponentShape shape, glm::vec3 start,
						   glm::vec3 end, glm::vec3 *finalCorrectedPosition,
						   bool *isOnGround, glm::vec3 *normal,
						   glm::vec3 *groundNormal,
						   float stepHeight,
						   float minNormalYcomponent) const
{
	if (stepHeight > shape.height) {
		stepHeight = shape.height;
	}
	
	if (normal) {
		*normal = glm::normalize(end - start);
	}
	
// 	bool wasOnGround = false;
	if (isOnGround) {
// 		wasOnGround = *isOnGround;
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

	Collision3D::Cylinder cyl{shape.height - stepHeight, shape.width*0.5f};

	bool hasCollision = false;
	
	float maxValidTravelDistanceFactor = 1.0f;
	
	Collision3D::RayInfo movement;
	movement.Calc(start + stepVector, end + stepVector);
	
	// Test strainght along movement vector
	
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
					if (normal)
						*normal = _normal;
				}
			}
		} else {
			LOG_FATAL("Static entity %lu does not have ComponentStaticTransform nor but is inside CollisionWorld",
					  e.id());
		}
	}
	

	
	*finalCorrectedPosition = start + (end - start) * maxValidTravelDistanceFactor;
	

	
	float maxOffsetHeight = 1.0f;
	glm::vec3 hitNormal;
	bool hasHitOnGround = false;
	
	Collision3D::RayInfo toGroundRay;
	toGroundRay.Calc(*finalCorrectedPosition + stepVector * 2.0f, *finalCorrectedPosition - stepVector);
	
	// Test on ground
	
	printf("Testing collision with %lu entities \n", count);
	
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
			glm::vec3 norm;
			printf("Testing ray: %f %f %f -> %f %f %f\n",
					toGroundRay.start.x,
					toGroundRay.start.y,
					toGroundRay.start.z,
					toGroundRay.end.x,
					toGroundRay.end.y,
					toGroundRay.end.z);
			bool res = false;
			if (s->shape.RayTest(t->trans, toGroundRay, near, norm)) {
				res = true;
				hasHitOnGround = true;
				if (near < maxOffsetHeight) {
					maxOffsetHeight = near;
					hitNormal = norm;
				}
			}
			printf("result %s: near: %f,    norm: %f %f %f\n",
					res ? "ON_GROUND" : "IN_AIR",
					near,
					norm.x,
					norm.y,
					norm.z);
			
		} else {
			LOG_FATAL("Static entity %lu does not have ComponentStaticTransform nor but is inside CollisionWorld",
					  e.id());
		}
	}
	
	hitNormal = glm::normalize(hitNormal);

	if (hasHitOnGround && (hitNormal.y >= minNormalYcomponent)) {// && (wasOnGround || (end.y >= start.y))) {
		hasCollision = true;
		*finalCorrectedPosition = end = toGroundRay.start + toGroundRay.dir * maxOffsetHeight;
		if (isOnGround) {
			*isOnGround = true;
		}
		if (groundNormal) {
			*groundNormal = hitNormal;
		}
	}
	
	return hasCollision;
}
