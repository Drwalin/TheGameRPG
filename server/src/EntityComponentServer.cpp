#include <flecs.h>

#include "../include/EntityComponentsServer.hpp"

int RegisterEntityComponentsServer(flecs::world &ecs)
{
	ecs.component<ComponentPlayerConnectionPeer>();
	return 0;
}
