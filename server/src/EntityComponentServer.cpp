#include <flecs.h>

#include <icon7/Debug.hpp>

#include "../include/EntityComponentsServer.hpp"

int RegisterEntityComponentsServer(flecs::world &ecs) {
	LOG_INFO("Registering components");
	ecs.component<ComponentPlayerConnectionPeer>();
	LOG_INFO("Done");
	return 0;
}
