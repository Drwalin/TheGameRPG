#include <flecs.h>

#include "../../common/include/RegistryComponent.inl.hpp"

#include "../include/EntityComponentsServer.hpp"

int RegisterEntityComponentsServer(flecs::world &ecs)
{
	REGISTER_COMPONENT_NO_SERIALISATION(ecs, TagNonPlayerEntity);
	REGISTER_COMPONENT_NO_SERIALISATION(ecs, TagPlayerEntity);

	REGISTER_COMPONENT_NO_SERIALISATION(ecs, ComponentPlayerConnectionPeer);

	return 0;
}
