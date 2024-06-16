#pragma once

#include <flecs.h>

#include "EntityComponents.hpp"

class Realm;

namespace EntitySystems
{
void UpdateMovement(
	Realm *realm, flecs::entity entity, const ComponentShape shape,
	ComponentMovementState &currentState,
	const ComponentLastAuthoritativeMovementState &lastAuthoritativeState,
	const ComponentMovementParameters &movementParams);
};
