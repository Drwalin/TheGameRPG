#pragma once

#include <flecs.h>

class Realm;
class ComponentShape;
class ComponentMovementState;
class ComponentLastAuthoritativeMovementState;
class ComponentMovementParameters;

namespace EntitySystems
{
void UpdateMovement(Realm *realm, flecs::entity entity,
					const class ::ComponentShape shape,
					class ::ComponentMovementState &currentState,
					const class ::ComponentLastAuthoritativeMovementState
						&lastAuthoritativeState,
					const class ::ComponentMovementParameters &movementParams);
};
