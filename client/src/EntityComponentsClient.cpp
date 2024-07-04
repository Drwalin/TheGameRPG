#include <flecs.h>

#include <icon7/Debug.hpp>

#include "../include/EntityComponentsClient.hpp"

int RegisterEntityComponents(flecs::world &ecs);

int RegisterEntityComponentsClient(flecs::world &ecs)
{
	LOG_INFO("Registering components");
	RegisterEntityComponents(ecs);
	ecs.component<ComponentMovementHistory>();
	LOG_INFO("Done");
	return 0;
}
