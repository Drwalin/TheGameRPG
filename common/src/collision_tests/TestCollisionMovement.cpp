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
					hasCollision = true;
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
	

	cyl.height = shape.height;
	
	float maxOffsetHeight = -stepHeight;
	glm::vec3 hitNormal = {0,0,0};
	bool hasHitOnGround = false;
	
	// Test on ground
	
	if (start.y < end.y && wasOnGround == false) {
// 		printf("Not checking is on ground: start.y: %f        end.y: %f\n", start.y, end.y);
		return hasCollision;
	}
	
	for (int i=0; i<count; ++i) {
		flecs::entity e = entities[i];
		if (auto *t = e.try_get<ComponentStaticTransform>()) {
			auto *s = e.try_get<ComponentCollisionShape>();
			if (s == nullptr) {
				LOG_FATAL("Static entity %lu does not have ComponentCollisionShape but is inside CollisionWorld",
						  e.id());
				continue;
			}
			
			float offsetHeight = 0;
			glm::vec3 onGroundNormal;
			if(s->shape.CylinderTestOnGround(t->trans, cyl, *finalCorrectedPosition, offsetHeight, &onGroundNormal)) {
// 				printf("a: |%f| <= %f ?       trans h: %f      charPos.y: %f\n", offsetHeight, stepHeight, t->trans.pos.y, finalCorrectedPosition->y);
				if (fabs(offsetHeight) <= stepHeight) {
// 					printf("b\n");
					if (maxOffsetHeight <= offsetHeight) {
// 						printf("c\n");
						hasHitOnGround = true;
						maxOffsetHeight = offsetHeight;
						hitNormal = onGroundNormal;
						
						assert(fabs(maxOffsetHeight) <= stepHeight);
					}
				}
			} else {
// 				printf("FAiled cylinder on ground test\n");
			}
		} else {
			LOG_FATAL("Static entity %lu does not have ComponentStaticTransform nor but is inside CollisionWorld",
					  e.id());
		}
	}
	
	if (count == 0) {
// 		printf("No collision objects\n");
	} else {
// 		printf("NUMBER OF POTENTIAL COLLIDING OBJECTS: %lu\n", count);
	}
	
	if (hasHitOnGround) {
		hitNormal = glm::normalize(hitNormal);
	}
	
	assert(fabs(maxOffsetHeight) <= stepHeight);
	
	assert(glm::length(hitNormal) < 1.1);
	
	if (hasHitOnGround && (hitNormal.y >= minNormalYcomponent)) {// && (wasOnGround || (end.y >= start.y))) {
		if (!wasOnGround) {
			if (start.y > end.y) {
				if (maxOffsetHeight > 0) {
					return hasCollision;
				}
			}
		}
		
		if (hasCollision == false) {
			if (normal) {
				*normal = hitNormal;
			}
		}
		hasCollision = true;
		finalCorrectedPosition->y -= maxOffsetHeight;
		end = *finalCorrectedPosition;
// 		printf("CORRECTED FINAL POSITION WHEN IS ON GROUND $$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$ maxOffsetHeight: %f,    isOnGround: %s\n", maxOffsetHeight, isOnGround?"YES":"NO");
		if (isOnGround) {
			*isOnGround = true;
		}
		if (groundNormal) {
			*groundNormal = hitNormal;
		}
	}
	fflush(stdout);
	
	return hasCollision;
}
