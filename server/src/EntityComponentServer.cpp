#include <flecs.h>

#include <icon7/Debug.hpp>

#include "../include/EntityComponentsServer.hpp"

int RegisterEntityComponentsServer(flecs::world &ecs)
{
	ecs.component<ComponentPlayerConnectionPeer>();
	return 0;
}
