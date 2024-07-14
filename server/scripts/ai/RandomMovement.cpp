#include <glm/ext/quaternion_trigonometric.hpp>
#include <glm/matrix.hpp>
#include <glm/mat4x4.hpp>
#include <glm/gtc/quaternion.hpp>

#include <flecs.h>

#include "../../../common/include/EntityComponents.hpp"
#include "../../include/ComponentCallbackRegistry.hpp"
#include "../../include/EntityGameComponents.hpp"
#include "../../include/EntityComponentsServer.hpp"
#include "../../../common/include/EntitySystems.hpp"
#include "../../include/SharedObject.hpp"
#include "../../include/RealmServer.hpp"
#include "../../include/ServerCore.hpp"

static void AiBehaviorTick_RandomWalk(RealmServer *realm, uint64_t entityId) {
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
	entity.set<ComponentMovementState>(currentState);
	
	
	if (state.onGround) {
		if (state.vel.length() < 1.0) {
			state.vel = glm::vec3(0, 0, params.maxMovementSpeedHorizontal);
		}
	
		float angle = state.rot.y += 0.15;
		
		glm::quat rot = glm::angleAxis(angle, glm::vec3(0,1,0));
		
		glm::vec4 V = glm::mat4_cast(rot) * glm::vec4(0, 0, params.maxMovementSpeedHorizontal, 0);
		state.vel = {V.x, V.y, V.z};
		
		lastState.oldState = state;
		entity.set(lastState);
	}
}

extern "C" void
Register_AiBehaviorTick_RandomWalk(class ServerCore *serverCore,
								   std::shared_ptr<SharedObject> sharedObject)
{
	named_callbacks::registry_entries::AiBehaviorTick::Set(
		"AIRandomMove", "AIRNDMV", &AiBehaviorTick_RandomWalk, sharedObject);
}
