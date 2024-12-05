#include "../../ICon7/include/icon7/Debug.hpp"

#include "../include/Realm.hpp"
#include "../include/EntityComponents.hpp"

#include "../include/EntitySystems.hpp"

namespace EntitySystems
{
void UpdateMovement(
	Realm *realm, flecs::entity entity, const ComponentShape shape,
	ComponentMovementState &currentState,
	const ComponentLastAuthoritativeMovementState &_lastAuthoritativeState,
	const ComponentMovementParameters &movementParams)
{
	LOG_TRACE("");
	ComponentMovementState prev = currentState;
	int I = 0;
	for (;; ++I) {
	LOG_TRACE("");
		const ComponentMovementState lastAuthoritativeState =
			_lastAuthoritativeState.oldState;
		const int64_t currentTick = realm->timer.currentTick;
		ComponentMovementState prev = lastAuthoritativeState;
		if (currentState.timestamp > prev.timestamp + 10 || I > 0) {
			prev = currentState;
		} else {
			currentState = prev;
		}
		auto &next = currentState;

		const int64_t _dt = currentTick - prev.timestamp;
		if (_dt < realm->minMovementDeltaTicks) {
			if (I)
				return;
			else
				break;
		}

		const float dt = _dt >= 500 ? 0.5f : _dt * 0.001f;

		if (next.onGround == true && prev.vel.y < 0.01) {
	LOG_TRACE("");
			glm::vec3 vel = prev.vel;
			if (fabs(vel.x) + fabs(vel.z) + fabs(vel.y) < 0.0005) {
				next = prev;
				next.timestamp = currentTick;
				if (I)
					return;
				else
					break;
			}

			glm::vec3 pos;
			const glm::vec3 oldPos = prev.pos;
			const glm::vec3 movement = vel * dt;
			const glm::vec3 newPos = oldPos + movement;
			/*
			if (realm->collisionWorld.TestCollisionMovement(
					shape, oldPos, newPos, &pos, &next.onGround, nullptr, 4,
					movementParams.stepHeight, 0.7, 0.07)) {
			}
			*/
			
			/*
			if (realm->collisionWorld.TestCollisionMovementRays(
					shape, oldPos, newPos, &pos, &next.onGround, nullptr, 4,
					movementParams.stepHeight, 0.7, 8, 0.1)) {
				*/
			if (realm->collisionWorld.TestCollisionMovementCylinder(
					shape, oldPos, newPos, &pos, &next.onGround, nullptr,
					movementParams.stepHeight, 0.7)) {
				float a = glm::length(pos-oldPos);
				float b = glm::length(newPos-oldPos);
				float dt2 = a / b * dt;
				if (a / b * dt > 0.005) {
					next.timestamp = prev.timestamp + dt2 * 1000.0;
				} else {
					next.timestamp = currentTick;
				}
			} else {
				next.timestamp = currentTick;
			}

			if (next.onGround) {
				vel.y = 0;
			}
			

			next.pos = pos;
			next.vel = vel;
			next.rot = prev.rot;

		} else {
	LOG_TRACE("");
			next.onGround = false;
			const glm::vec3 acc = {0, realm->gravity, 0};

			glm::vec3 vel = prev.vel + acc * dt * 0.5f;

			const glm::vec3 oldPos = prev.pos;
			const glm::vec3 movement = vel * dt;
			const glm::vec3 newPos = oldPos + movement;

			// test collision here:
			glm::vec3 pos;
			glm::vec3 normal;
			if (realm->collisionWorld.TestCollisionMovementCylinder(
					shape, oldPos, newPos, &pos, &next.onGround, &normal,
					movementParams.stepHeight, 0.7)) {
				float a = glm::length(pos-oldPos);
				float b = glm::length(newPos-oldPos);
				float dt2 = a / b * dt;
				if (a / b * dt > 0.005) {
					next.timestamp = prev.timestamp + dt2 * 1000.0;
				} else {
					next.timestamp = currentTick;
				}
				vel = prev.vel + acc * dt2 * 0.5f;
				normal = glm::normalize(normal);
				vel -= normal * glm::dot(normal, vel);
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
			} else {
				next.timestamp = currentTick;
			}
			vel += acc * dt * 0.5f;

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
	LOG_TRACE("");
	}
	LOG_TRACE("");

	glm::vec3 d = currentState.pos - prev.pos;
	if (glm::dot(d, d) > 0.00001) {
		realm->collisionWorld.UpdateEntityBvh(entity, shape, currentState.pos);
	}
}
} // namespace EntitySystems
