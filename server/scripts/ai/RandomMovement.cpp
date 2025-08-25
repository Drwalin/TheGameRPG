#define GLM_ENABLE_EXPERIMENTAL
#include "../../thirdparty/Collision3D/SpatialPartitioning/glm/glm/ext/quaternion_trigonometric.hpp"
#include "../../thirdparty/Collision3D/SpatialPartitioning/glm/glm/gtc/quaternion.hpp"

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

#if not __unix__
#define M_PI 3.141592653589793
#else
#endif

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
		FILTER_CHARACTER);

	uint64_t found = 0;
	glm::vec3 targetPos;

	glm::vec3 eyes = state->pos;
	eyes.y += shape->height;
	glm::vec3 hitPos = {100000, 100000, 100000};
	uint64_t targetHitId = 0;

	for (uint64_t id : entities) {
		if (id != entityId) {
			flecs::entity target = realm->Entity(id);
			if (target.has<ComponentPlayerConnectionPeer>() &&
				target.has<ComponentMovementState>() &&
				target.has<ComponentShape>()) {
				const ComponentMovementState *s =
					target.try_get<ComponentMovementState>();
				const ComponentShape *sh = target.try_get<ComponentShape>();

				glm::vec3 _hitPos;
				uint64_t _targetHitId = 0;
				if (realm->collisionWorld.RayTestFirstHit(
						eyes, s->pos + glm::vec3(0, sh->height * 0.5f, 0),
						&_hitPos, nullptr, &_targetHitId, nullptr, nullptr,
						entityId)) {
					if (_targetHitId == id) {
						if (found) {
							if (glm::length2(state->pos - targetPos) >
								glm::length2(state->pos - s->pos)) {
								found = id;
								targetPos = s->pos;
								targetHitId = _targetHitId;
								hitPos = _hitPos;
							}
						} else {
							found = id;
							targetPos = s->pos;
							targetHitId = _targetHitId;
							hitPos = _hitPos;
						}
					}
				}
			}
		}
	}

	if (found != 0) {
		MoveTo(state, params, targetPos, acceptableDistance);

		if (rand() % 10 == 0) {
			if (glm::length2(eyes - hitPos) < 10.0f) {
				ComponentLastAuthoritativeMovementState ls{*state};
				realm->Attack(entityId, ls, targetHitId, hitPos, 0, 0, 0);
				realm->Attack(entityId, ls, targetHitId, hitPos, 0, 0, 1);
				// LOG_INFO("Attack %lu->%lu", entityId, targetHitId);
			} else {
				// LOG_INFO("Miss %lu->%lu     dist: %.2f", entityId,
				// targetHitId, glm::length(eyes - hitPos));
			}
		}
		return true;
	}
	return false;
}

static void AiBehaviorTick_RandomWalk(RealmServer *realm, uint64_t entityId)
{
	flecs::entity entity = realm->Entity(entityId);
	auto lastState = *entity.try_get<ComponentLastAuthoritativeMovementState>();
	auto state = *entity.try_get<ComponentMovementState>();
	auto params = *entity.try_get<ComponentMovementParameters>();

	const ComponentShape *shape = entity.try_get<ComponentShape>();
	if (shape == nullptr) {
		return;
	}

	ComponentMovementState currentState = lastState.oldState;
	auto movementParams = entity.try_get<ComponentMovementParameters>();
	EntitySystems::UpdateMovement(realm, entity, *shape, currentState,
								  lastState, *movementParams);

	if (state.onGround) {
		// TODO: Replace 3.0f and 15.0f with apropriate values from some
		//       component with parameters
		if (false == TryFollowingPlayer(realm, entityId, &state, shape,
										movementParams, 3.0f, 15.0f)) {
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
	}
	entity.set<ComponentLastAuthoritativeMovementState>({state});
}

extern "C" void
Register_AiBehaviorTick_RandomWalk(class ServerCore *serverCore,
								   std::shared_ptr<SharedObject> sharedObject)
{
	named_callbacks::registry_entries::AiBehaviorTick::Set(
		"AIRandomMove", "AIRNDMV", &AiBehaviorTick_RandomWalk, sharedObject);
}
