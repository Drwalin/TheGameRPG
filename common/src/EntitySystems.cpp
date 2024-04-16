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
	}
	auto &next = currentState;
	bool onGround = next.onGround;

	int64_t _dt = currentTick - prev.timestamp;
	if (_dt < realm->minDeltaTicks) {
		return;
	}

	float dt = _dt * 0.001f;

	{
		glm::vec3 vel = prev.vel;
		glm::vec3 pos = prev.pos;
		DEBUG("entity=%lu    Vel = (%f, %f, %f),   dt: %li >= %i,    pos = (%f "
			  "%f %f)",
			  entity.id(), vel.x, vel.y, vel.z, _dt, realm->minDeltaTicks,
			  pos.x, pos.y, pos.z);
	}

	if (onGround == true && prev.vel.y < 0.1) {
		glm::vec3 vel = prev.vel;
		vel.y = 0;
		if (fabs(vel.x) + fabs(vel.z) < 0.005) {
			next = prev;
			next.timestamp = currentTick;
			return;
		}

		// step stepping algorithm:

		// step 1: up
		glm::vec3 oldPos = prev.pos;
		glm::vec3 newPos = oldPos + glm::vec3(0, movementParams.stepHeight, 0);
		glm::vec3 pos;
		realm->collisionWorld.TestCollisionMovement(shape, oldPos, newPos, &pos,
													nullptr, nullptr);
		newPos = pos;

		// step 2: vertical
		glm::vec3 movement = vel * dt;
		float heightDiff = newPos.y - oldPos.y;
		oldPos = newPos;
		newPos = oldPos + movement;
		realm->collisionWorld.TestCollisionMovement(shape, oldPos, newPos, &pos,
													nullptr, nullptr);
		newPos = pos;

		// step 3: down
		oldPos = newPos;
		newPos =
			oldPos - glm::vec3(0, heightDiff + movementParams.stepHeight, 0);
		bool isOnGround = false;
		if (realm->collisionWorld.TestCollisionMovement(
				shape, oldPos, newPos, &pos, &isOnGround, nullptr)) {
			onGround = isOnGround;
		}

		if (onGround) {
			vel.y = 0;
		}

		next.timestamp = currentTick;
		next.pos = pos;
		next.vel = vel;
		next.rot = prev.rot;
		next.onGround = onGround;

		glm::vec3 p1 = prev.pos, p2 = next.pos;

		DEBUG("Step-up position (%lu) dt(%f): (%f, %f, %f) -> (%f, %f, %f)",
			  entity.id(), dt, p1.x, p1.y, p1.z, p2.x, p2.y, p2.z);
	} else {
		glm::vec3 acc = {0, realm->gravity, 0};

		glm::vec3 vel = prev.vel; // + acc * dt;

		// 		DEBUG("Vel = (%f, %f, %f)", vel.x, vel.y, vel.z);

		if (vel.y < -50) {
			vel.y = -50;
		}
		if (vel.y > 50) {
			vel.y = 50;
		}

		glm::vec3 oldPos = prev.pos;
		glm::vec3 movement = prev.vel * dt; // + acc * (dt * dt * 0.5f);
		glm::vec3 newPos = oldPos + movement;

		// 		{
		// 			glm::vec3 m = movement;
		// 			DEBUG("movement = (%f, %f, %f)", m.x, m.y, m.z);
		// 		}
		//
		// 		{
		// 			glm::vec3 m = acc;
		// 			DEBUG("acc = (%f, %f, %f)", m.x, m.y, m.z);
		// 		}

		// test collision here:
		glm::vec3 pos;
		bool isOnGround = false;
		glm::vec3 normal;
		if (realm->collisionWorld.TestCollisionMovement(
				shape, oldPos, newPos, &pos, &isOnGround, &normal)) {
			if (vel.y <= 1) {
				onGround = isOnGround;
			}
			normal = glm::normalize(normal);
			glm::vec3 v = normal * glm::dot(normal, vel);
			vel -= v;
			{
				glm::vec3 m = normal;
				DEBUG("normal = (%f, %f, %f)", m.x, m.y, m.z);
			}
		} else {
			vel = prev.vel + acc * dt;
		}

		next.timestamp = currentTick;
		next.pos = pos;
		next.vel = vel;
		next.rot = prev.rot;
		next.onGround = onGround;

		glm::vec3 p1 = prev.pos, p2 = next.pos;

		DEBUG("new Vel = (%f, %f, %f)", vel.x, vel.y, vel.z);
		DEBUG("Falling position (%lu) dt(%f): (%f, %f, %f) -> (%f, %f, %f)",
			  entity.id(), dt, p1.x, p1.y, p1.z, p2.x, p2.y, p2.z);
		printf("\n");
	}

	glm::vec3 d = currentState.pos - prev.pos;
	if (glm::dot(d, d) > 0.00001) {
		realm->collisionWorld.UpdateEntityBvh(entity.id(), shape,
											  currentState.pos);
	}
}
} // namespace EntitySystems
