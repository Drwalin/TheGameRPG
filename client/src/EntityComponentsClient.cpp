#include <flecs.h>

#include <icon7/Debug.hpp>

#include "../include/EntityComponentsClient.hpp"

int RegisterEntityComponents(flecs::world &ecs);

int RegisterEntityComponentsClient(flecs::world &ecs)
{
	RegisterEntityComponents(ecs);
	ecs.component<ComponentMovementHistory>();
	return 0;
}
