#include <flecs.h>

#include <icon7/Debug.hpp>

#include "EntityComponentsFrontend.hpp"

int RegisterEntityComponentsClient(flecs::world &ecs);

int RegisterFrontendEntityComponents(flecs::world &ecs)
{
	RegisterEntityComponentsClient(ecs);
	ecs.component<ComponentGodotNode>();
	ecs.component<ComponentStaticGodotNode>();
	return 0;
}
