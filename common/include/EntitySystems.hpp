#pragma once

#include "../../thirdparty/flecs/distr/flecs.h"

class Realm;
struct ComponentShape;
struct ComponentMovementState;
struct ComponentLastAuthoritativeMovementState;
struct ComponentMovementParameters;

namespace EntitySystems
{
void UpdateMovement(Realm *realm, flecs::entity entity,
					const struct ::ComponentShape shape,
					struct ::ComponentMovementState &currentState,
					const struct ::ComponentLastAuthoritativeMovementState
						&lastAuthoritativeState,
					const struct ::ComponentMovementParameters &movementParams);
};
