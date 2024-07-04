#include <flecs.h>

#include <icon7/Debug.hpp>

#include "EntityComponentsFrontend.hpp"

int RegisterEntityComponentsClient(flecs::world &ecs);

int RegisterFrontendEntityComponents(flecs::world &ecs)
{
	LOG_INFO("Registering components");
	RegisterEntityComponentsClient(ecs);
	ecs.component<ComponentGodotNode>();
	ecs.component<ComponentStaticGodotNode>();
	LOG_INFO("Done");
	return 0;
}
