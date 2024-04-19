#pragma once

#include <flecs.h>

#include "EntityData.hpp"

class Realm;

namespace EntitySystems
{
void UpdateMovement(
	Realm *realm, flecs::entity entity, const EntityShape shape,
	EntityMovementState &currentState,
	const EntityLastAuthoritativeMovementState &lastAuthoritativeState,
	const EntityMovementParameters &movementParams);
};
