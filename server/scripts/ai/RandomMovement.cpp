#define GLM_ENABLE_EXPERIMENTAL
#include <glm/ext/quaternion_trigonometric.hpp>
#include <glm/matrix.hpp>
#include <glm/mat4x4.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/vector_angle.hpp>
#include <glm/gtx/quaternion.hpp>

#include <flecs.h>

#include "../../../common/include/EntityComponents.hpp"
#include "../../include/ComponentCallbackRegistry.hpp"
#include "../../include/ComponentCallbacks.hpp"
#include "../../../common/include/EntityComponents.hpp"
#include "../../include/EntityComponentsServer.hpp"
#include "../../../common/include/EntitySystems.hpp"
#include "../../include/SharedObject.hpp"
#include "../../include/RealmServer.hpp"
#include "../../include/ServerCore.hpp"
#include "../../include/callbacks/CallbackAiBehaviorTick.hpp"

static void MoveInDirectionForward(ComponentMovementState *state,
								   const ComponentMovementParameters *params,
								   glm::vec3 direction)
{
	glm::vec3 flatDir = direction;
	flatDir.y = 0;

	float angle = -atan2(flatDir.z, flatDir.x) + atan2(1, 0);

	if (angle < 0.0) {
		angle += 2.0 * M_PI;
	}

	state->rot.y = angle;

	state->vel = glm::normalize(flatDir) *
				 (params->maxMovementSpeedHorizontal * 1.6f / 5.0f);
}

static void MoveTo(ComponentMovementState *state,
				   const ComponentMovementParameters *params, glm::vec3 target,
				   float acceptableDistance)
{
	MoveInDirectionForward(state, params, target - state->pos);

	if (glm::length2(state->pos - target) <=
		acceptableDistance * acceptableDistance) {
		state->vel = {0, 0, 0};
		return;
	}
}

static bool TryFollowingPlayer(RealmServer *realm, uint64_t entityId,
							   ComponentMovementState *state,
							   const ComponentShape *shape,
							   const ComponentMovementParameters *params,
							   float acceptableDistance, float searchDistance)
{
	std::vector<uint64_t> entities;
	realm->collisionWorld.TestForEntitiesSphere(
		state->pos, searchDistance, &entities,
		CollisionWorld::FILTER_CHARACTER);

	uint64_t found = 0;
	glm::vec3 targetPos;

	glm::vec3 eyes = state->pos;
	eyes.y += shape->height;

	for (uint64_t id : entities) {
		if (id != entityId) {
			flecs::entity target = realm->Entity(id);
			if (target.has<ComponentPlayerConnectionPeer>() &&
				target.has<ComponentMovementState>() &&
				target.has<ComponentShape>()) {
				const ComponentMovementState *s =
					target.get<ComponentMovementState>();
				const ComponentShape *sh = target.get<ComponentShape>();

				uint64_t targetHit = 0;
				if (realm->collisionWorld.RayTestFirstHit(
						eyes, s->pos + glm::vec3(0, sh->height * 0.5f, 0),
						nullptr, nullptr, &targetHit, nullptr, nullptr,
						entityId)) {
					if (targetHit == id) {
						if (found) {
							if (glm::length2(state->pos - targetPos) >
								glm::length2(state->pos - s->pos)) {
								found = id;
								targetPos = s->pos;
							}
						} else {
							found = id;
							targetPos = s->pos;
						}
					}
				}
			}
		}
	}

	if (found != 0) {
		MoveTo(state, params, targetPos, acceptableDistance);
		return true;
	}
	return false;
}

static void AiBehaviorTick_RandomWalk(RealmServer *realm, uint64_t entityId)
{
	flecs::entity entity = realm->Entity(entityId);
	auto lastState = *entity.get<ComponentLastAuthoritativeMovementState>();
	auto state = *entity.get<ComponentMovementState>();
	auto params = *entity.get<ComponentMovementParameters>();

	const ComponentShape *shape = entity.get<ComponentShape>();
	if (shape == nullptr) {
		return;
	}

	ComponentMovementState currentState = lastState.oldState;
	auto movementParams = entity.get<ComponentMovementParameters>();
	EntitySystems::UpdateMovement(realm, entity, *shape, currentState,
								  lastState, *movementParams);

	if (state.onGround) {
		// TODO: Replace 5.0f and 60.0f with apropriate values from some
		// component with parameters
		if (false == TryFollowingPlayer(realm, entityId, &state, shape,
										movementParams, 5.0f, 15.0f)) {
			state.rot.y += 0.15;
			float angle = state.rot.y;
			glm::quat rot = glm::angleAxis(angle, glm::vec3(0, 1, 0));
			if (state.vel.length() < 1.0) {
				state.vel = glm::vec3(0, 0, params.maxMovementSpeedHorizontal);
			}
			glm::vec4 V = glm::mat4_cast(rot) *
						  glm::vec4(0, 0, params.maxMovementSpeedHorizontal, 0);
			state.vel = {V.x, V.y, V.z};
		}

		lastState.oldState = state;
		entity.set<ComponentLastAuthoritativeMovementState>(lastState);
	} else {
		lastState.oldState = state;
		entity.set<ComponentLastAuthoritativeMovementState>(lastState);
	}
}

extern "C" void
Register_AiBehaviorTick_RandomWalk(class ServerCore *serverCore,
								   std::shared_ptr<SharedObject> sharedObject)
{
	named_callbacks::registry_entries::AiBehaviorTick::Set(
		"AIRandomMove", "AIRNDMV", &AiBehaviorTick_RandomWalk, sharedObject);
}
