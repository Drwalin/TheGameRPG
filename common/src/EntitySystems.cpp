#include "../include/Realm.hpp"
#include "../include/EntityComponents.hpp"

#include "../include/EntitySystems.hpp"

extern Collision3D::RampRectangle RAMP_RECT_GLOB;
extern Collision3D::Transform RAMP_RECT_GLOB_TRANS;

extern bool DO_PRINT;
bool DO_PRINT = false;
#define printf(...) {if(DO_PRINT){printf(__VA_ARGS__);}}

#define PRINT_TRANS(POS) RAMP_RECT_GLOB_TRANS.ToLocal(POS).x, RAMP_RECT_GLOB_TRANS.ToLocal(POS).y, RAMP_RECT_GLOB_TRANS.ToLocal(POS).z 


namespace EntitySystems
{
	
	
void UpdateMovement(Realm *realm, flecs::entity entity,
					const ComponentShape shape,
					ComponentMovementState &currentState,
					const ComponentMovementState prev,
					const ComponentMovementParameters &movementParams,
					float tickFactor, bool updateState)
{
	DO_PRINT = updateState;
	auto &next = currentState;

	// printf("[%5li] : %.2f %.2f %.2f\n", entity.id(), next.pos.x, next.pos.y,
	// next.pos.z);

	const float dt = Realm::TICK_DURATION_SECONDS * tickFactor;

	if (next.onGround == true && prev.vel.y < 0.01) {
		glm::vec3 vel = prev.vel;
		if (fabs(vel.x) + fabs(vel.z) + fabs(vel.y) < 0.0005) {
			next = prev;
			return;
		}

		printf("\n");
		printf("\n");

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
			printf("old pos = %7.2f %7.2f %7.2f\n", PRINT_TRANS(prev.pos));
			printf("new pos = %7.2f %7.2f %7.2f\n", PRINT_TRANS(pos));
			pos += normal * 0.01f;
			printf("new pos with norm correction = %7.2f %7.2f %7.2f", PRINT_TRANS(pos));
			float lenM = glm::length((prev.pos-pos)*glm::vec3(1,0,1));
			printf("        (len: %.2f)       (tick: %lu)\n", lenM, realm->timer.currentTick.v);
		} else {
			float lenM = glm::length((prev.pos-pos)*glm::vec3(1,0,1));
			printf("old pos = %7.2f %7.2f %7.2f\n", PRINT_TRANS(prev.pos));
			printf("new pos = %7.2f %7.2f %7.2f", PRINT_TRANS(pos));
			printf("        (len: %.2f)       (tick: %lu)\n", lenM, realm->timer.currentTick.v);
		}

		if (next.onGround) {
			vel.y = 0;
		}
		
		

		next.pos = pos;
		next.vel = vel;
		next.rot = prev.rot;

	} else {

		printf("\n");
		printf("\n");

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
			
		float lenM = glm::length((prev.pos-pos)*glm::vec3(1,0,1));
		printf("old pos = %7.2f %7.2f %7.2f\n", PRINT_TRANS(prev.pos));
		printf("new pos = %7.2f %7.2f %7.2f", PRINT_TRANS(pos));
		printf("        (len: %.2f)       (tick: %lu)\n", lenM, realm->timer.currentTick.v);

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
	DO_PRINT = false;
}
} // namespace EntitySystems
