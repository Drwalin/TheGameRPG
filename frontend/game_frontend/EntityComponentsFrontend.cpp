#include "../../thirdparty/flecs/distr/flecs.h"

#include "EntityComponentsFrontend.hpp"

int RegisterEntityComponentsClient(flecs::world &ecs);

int RegisterFrontendEntityComponents(flecs::world &ecs)
{
	RegisterEntityComponentsClient(ecs);
	ecs.component<ComponentGodotNode>();
	ecs.component<ComponentStaticGodotNode>();
	return 0;
}
