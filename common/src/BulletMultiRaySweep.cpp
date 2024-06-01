#include <bullet/LinearMath/btVector3.h>
#include <bullet/btBulletCollisionCommon.h>
#include <bullet/BulletDynamics/Character/btKinematicCharacterController.h>

#include "../include/CollisionWorld.hpp"

#if not __unix__
# define M_PI 3.141292
#else
#endif

/*
 * verticalRayTestToGroundDistance - if <0 then casts only one ray at
 * destination ignored for now horizontalFromCenterCorrectionRaysCount - should
 * be at least 4
 */
bool CollisionWorld::TestCollisionMovementRays(
	EntityShape shape, glm::vec3 start, glm::vec3 end,
	glm::vec3 *finalCorrectedPosition, bool *isOnGround, glm::vec3 *normal,
	int horizontalRaysCountInMovementDirection, float stepHeight,
	float minNormalYcomponent, int horizontalFromCenterCorrectionRaysCount,
	float verticalRayTestToGroundDistance) const
{
	if (normal) {
		*normal = glm::normalize(end - start);
	}
	bool wasOnGround = false;
	if (isOnGround) {
		wasOnGround = *isOnGround;
		*isOnGround = false;
	}

	const float maxSingleCorrectionLength = shape.width * 0.25;

	horizontalRaysCountInMovementDirection =
		glm::max(horizontalRaysCountInMovementDirection, 3);
	horizontalRaysCountInMovementDirection =
		glm::min(horizontalRaysCountInMovementDirection, 64);
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

	const float heightStart = stepHeight;
	const float heightEnd = shape.height - 0.001f;
	const float heightDiff = heightEnd - heightStart;
	const float heightRaysDistance =
		heightDiff / (horizontalRaysCountInMovementDirection - 1);

	const glm::vec3 travelDirNorm = glm::normalize(end - start);

	glm::vec3 endWithMargin = end + travelDirNorm * maxSingleCorrectionLength;

	// Test rays with travel direction on travel path
	for (int i = 0; i < horizontalRaysCountInMovementDirection; ++i) {
		const float h = heightStart + heightRaysDistance * i;
		const glm::vec3 offset = glm::vec3(0, h, 0);
		const glm::vec3 startVec = start + offset;
		const glm::vec3 dirVec = endWithMargin - start;
		glm::vec3 hp, n;
		float tf;
		if (RayTestFirstHitWithObjects(startVec, dirVec, &hp, &n, &tf,
									   objects)) {
			endWithMargin = hp - offset;
			if (normal) {
				if (glm::dot(n, end - start) < glm::dot(*normal, end - start)) {
					*normal = n;
				}
			}
		}
	}

	end = endWithMargin - travelDirNorm * maxSingleCorrectionLength;

	// Test cross rays from old knee to new head and the opposite
	{
		const glm::vec3 h[2]{{0, heightStart, 0}, {0, shape.height, 0}};
		for (int i = 0; i < 2; ++i) {
			const glm::vec3 startVec = start + h[i];
			const glm::vec3 endVec = end + h[(i + 1) % 2];
			const glm::vec3 dirVec = endVec - startVec;
			glm::vec3 hp, n;
			float tf;
			if (RayTestFirstHitWithObjects(startVec, dirVec, &hp, &n, &tf,
										   objects)) {
				end -= (end - start) * tf;
				end -= n * (maxSingleCorrectionLength * 0.25f);
				if (normal) {
					if (glm::dot(n, end - start) <
						glm::dot(*normal, end - start)) {
						*normal = n;
					}
				}
			}
		}
	}

	// TODO: test vertical rays on travel path

	bool hasCollision = false;
	// test horizontal 'crown' of rays outward to push out of walls
	{
		const float deltaAngle =
			2.0f * M_PI / horizontalFromCenterCorrectionRaysCount;
		const float cosa = cos(deltaAngle);
		const float sina = sqrt(1.0 - cosa * cosa); // deltaAngle must be
		glm::vec3 dir{1, 0, 0};
		glm::mat3 rotMatrix = {cosa, 0, -sina, 0, 0, 0, sina, 0, cosa};
		for (int k = 0; k < 2; ++k) { // perform double correction, remove in
									  // future?
			for (int j = 0; j < horizontalFromCenterCorrectionRaysCount; ++j) {
				if (j > 0) {
					dir = rotMatrix * dir;
				}
				glm::vec3 trueDir = dir * (maxSingleCorrectionLength * 1.1f);
				for (int i = 0; i < horizontalRaysCountInMovementDirection;
					 ++i) {
					const float h = heightStart + heightRaysDistance * i;
					const glm::vec3 offset = glm::vec3(0, h, 0);
					const glm::vec3 trueOrigin =
						end - dir * (maxSingleCorrectionLength * 0.1f);
					const glm::vec3 startVec = trueOrigin + offset;

					float tf;
					glm::vec3 n;
					if (RayTestFirstHitWithObjects(startVec, trueDir, nullptr,
												   &n, &tf, objects)) {
						hasCollision = true;
						if (tf <= 0.1f / 1.1f) {
							end += trueDir * (0.1f / 1.1f - tf);
						} else {
							end -= trueDir * (1.0f - tf);
						}
						if (normal) {
							if (glm::dot(n, end - start) <
								glm::dot(*normal, end - start)) {
								*normal = n;
							}
						}
					}
				}
			}
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
// 	const glm::vec3 toHeadStart = toMid - glm::vec3(0, 0.1, 0);

	glm::vec3 hitPoint, hitNormal;
	float travelFactor;

	// test for head collision
	// 	if (RayTestFirstHitWithObjects(end + toHeadStart, toHeadTop -
	// toHeadStart, 								   &hitPoint, &hitNormal, &travelFactor, 								   objects)) {
	// 		// TODO: correct position due to ceiling
	// 		if (normal) {
	// 			if (glm::dot(hitNormal, end-start) < glm::dot(*normal,
	// end-start)) { 				*normal = hitNormal;
	// 			}
	// 		}
	// 	}

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
