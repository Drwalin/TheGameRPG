#include <icon7/Debug.hpp>

#include "../include/Realm.hpp"

#include "../include/EntitySystems.hpp"

namespace EntitySystems
{
void UpdateMovement(
	Realm *realm, flecs::entity entity, const EntityShape shape,
	EntityMovementState &currentState,
	const EntityLastAuthoritativeMovementState &_lastAuthoritativeState,
	const EntityMovementParameters &movementParams)
{
	const EntityMovementState lastAuthoritativeState =
		_lastAuthoritativeState.oldState;
	int64_t currentTick = realm->timer.currentTick;
	EntityMovementState prev = lastAuthoritativeState;
	if (currentState.timestamp >
		prev.timestamp + 10) { // < realm->ticksBeforeIgnoringInputMovement) {
		prev = currentState;
	} else {
		currentState = prev;
	}
	auto &next = currentState;

	int64_t _dt = currentTick - prev.timestamp;
	if (_dt < realm->minDeltaTicks) {
		return;
	}

	float dt = _dt * 0.001f;

	if (next.onGround == true && prev.vel.y < 0.01) {
		glm::vec3 vel = prev.vel;
		vel.y = 0;
		if (fabs(vel.x) + fabs(vel.z) + fabs(vel.y) < 0.005) {
			next = prev;
			next.timestamp = currentTick;
			return;
		}

		// step stepping algorithm:
		glm::vec3 pos;
		if (false) { // pure capsule sweep disabled
			// step 1: up
			glm::vec3 oldPos = prev.pos;
			glm::vec3 newPos =
				oldPos + glm::vec3(0, movementParams.stepHeight, 0);
			realm->collisionWorld.TestCollisionMovement(
				shape, oldPos, newPos, &pos, nullptr, nullptr, 4, 0, 0, 0.07);
			newPos = pos;

			// step 2: vertical
			glm::vec3 movement = vel * dt;
			float heightDiff = newPos.y - oldPos.y;
			oldPos = newPos;
			newPos = oldPos + movement;
			realm->collisionWorld.TestCollisionMovement(
				shape, oldPos, newPos, &pos, nullptr, nullptr, 4,
				movementParams.stepHeight, 0.7, 0.07);
			newPos = pos;

			// step 3: down
			oldPos = newPos;
			newPos = oldPos -
					 glm::vec3(0, heightDiff + movementParams.stepHeight, 0);
			if (realm->collisionWorld.TestCollisionMovement(
					shape, oldPos, newPos, &pos, &next.onGround, nullptr, 4,
					movementParams.stepHeight, 0.7, 0.07)) {
			}
		} else {
			next.onGround = false;

			glm::vec3 oldPos = prev.pos;
			glm::vec3 movement = vel * dt;
			glm::vec3 newPos = oldPos + movement;

			if (realm->collisionWorld.TestCollisionMovement(
					shape, oldPos, newPos, &pos, &next.onGround, nullptr, 4,
					movementParams.stepHeight, 0.7, 0.07)) {
			}
		}

		if (next.onGround) {
			vel.y = 0;
		}

		next.timestamp = currentTick;
		next.pos = pos;
		next.vel = vel;
		next.rot = prev.rot;

	} else {
		glm::vec3 acc = {0, realm->gravity, 0};

		glm::vec3 vel = prev.vel; // + acc * dt;

		if (vel.y < -50) {
			vel.y = -50;
		}
		if (vel.y > 50) {
			vel.y = 50;
		}

		glm::vec3 oldPos = prev.pos;
		glm::vec3 movement = prev.vel * dt; // + acc * (dt * dt * 0.5f);
		glm::vec3 newPos = oldPos + movement;

		// test collision here:
		glm::vec3 pos;
		glm::vec3 normal;
		if (realm->collisionWorld.TestCollisionMovement(
				shape, oldPos, newPos, &pos, &next.onGround, &normal, 4,
				movementParams.stepHeight, 0.7, 0.07)) {
			normal = glm::normalize(normal);
			glm::vec3 v = normal * glm::dot(normal, vel);
			vel -= v;
		} else {
			vel = prev.vel + acc * dt;
		}

		next.timestamp = currentTick;
		next.pos = pos;
		next.vel = vel;
		next.rot = prev.rot;
	}

	glm::vec3 d = currentState.pos - prev.pos;
	if (glm::dot(d, d) > 0.00001) {
		realm->collisionWorld.UpdateEntityBvh(entity.id(), shape,
											  currentState.pos);
	}
}
} // namespace EntitySystems
