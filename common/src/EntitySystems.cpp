#include "../include/Realm.hpp"
#include "../include/EntityComponents.hpp"

#include "../include/EntitySystems.hpp"

namespace EntitySystems
{
void UpdateMovement(Realm *realm, flecs::entity entity,
					const ComponentShape shape,
					ComponentMovementState &currentState,
					const ComponentMovementState prev,
					const ComponentMovementParameters &movementParams,
					float tickFactor, bool updateState)
{
	auto &next = currentState;

	// 	printf("[%5li] : %.2f %.2f %.2f\n", entity.id(), next.pos.x, next.pos.y,
	// next.pos.z);

	const float dt = Realm::TICK_DURATION_SECONDS * tickFactor;

	if (next.onGround == true && prev.vel.y < 0.01) {
		glm::vec3 vel = prev.vel;
		if (fabs(vel.x) + fabs(vel.z) + fabs(vel.y) < 0.0005) {
			next = prev;
			return;
		}

		glm::vec3 pos;
		const glm::vec3 oldPos = prev.pos;
		const glm::vec3 movement = vel * dt;
		const glm::vec3 newPos = oldPos + movement;

		glm::vec3 normal, groundNormal;
		if (realm->collisionWorld.TestCollisionMovement(
				shape, oldPos, newPos, &pos, &next.onGround, &normal,
				&groundNormal, movementParams.stepHeight, 0.7)) {
			if (glm::length2(pos - oldPos) < 0.00000001 && oldPos != newPos) {
				next.onGround = false;
			}
			pos += normal * 0.01f;
		}

		if (next.onGround) {
			vel.y = 0;
		}

		next.pos = pos;
		next.vel = vel;
		next.rot = prev.rot;

	} else {

		next.onGround = false;
		const glm::vec3 acc = {0, realm->gravity, 0};

		glm::vec3 vel = prev.vel + acc * dt * 0.5f;

		const glm::vec3 oldPos = prev.pos;
		const glm::vec3 movement = vel * dt;
		const glm::vec3 newPos = oldPos + movement;
		vel += acc * dt * 0.5f;

		// test collision here:
		glm::vec3 pos;
		glm::vec3 normal;
		glm::vec3 groundNormal;
		if (realm->collisionWorld.TestCollisionMovement(
				shape, oldPos, newPos, &pos, &next.onGround, &normal,
				&groundNormal, movementParams.stepHeight, 0.7)) {
			normal = glm::normalize(normal);

			if (/*next.onGround == false &&*/ glm::dot(vel, normal) < 0) {
				vel -= normal * glm::dot(normal, vel);
			}
			if (next.onGround) { // && glm::dot(vel, groundNormal) < 0) {
				vel -= groundNormal * glm::dot(groundNormal, vel);
				vel.y = 0;
			}

			if (glm::length2(pos - oldPos) < 0.00000001 && oldPos != newPos) {
				next.onGround = false;
			}
			pos += normal * 0.01f;

			/*
			if (vel.y > 0) {
				glm::vec3 vv = vel;
				vv.y = 0.0f;
				float vvl = glm::length(vv);
				float frictionFactor = 0.5 * dt;
				if (frictionFactor >= vvl) {
					vel = {0, 0, 0};
				} else {
					vel -= vv * (frictionFactor / vvl);
				}
			}
			*/
		}

		if (vel.y < -50) {
			vel.y = -50;
		}
		if (vel.y > 50) {
			vel.y = 50;
		}

		next.pos = pos;
		next.vel = vel;
		next.rot = prev.rot;
	}

	currentState = next;

	if (currentState.pos != prev.pos && updateState) {
		realm->collisionWorld.EntitySetTransform(entity, next.pos, shape);
	}
}
} // namespace EntitySystems
