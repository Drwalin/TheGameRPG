#include "../../thirdparty/flecs/distr/flecs.h"

#include "../include/EntityComponentsClient.hpp"

int RegisterEntityComponents(flecs::world &ecs);

int RegisterEntityComponentsClient(flecs::world &ecs)
{
	RegisterEntityComponents(ecs);
	ecs.component<ComponentMovementHistory>();
	ecs.component<ComponentLastAuthoritativeStateUpdateTime>();
	return 0;
}
