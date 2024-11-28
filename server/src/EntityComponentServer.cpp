#include <flecs.h>

#include "../../common/include/RegistryComponent.inl.hpp"

#include "../include/EntityComponentsServer.hpp"

int RegisterEntityComponentsServer(flecs::world &ecs)
{
	ecs.component<TagNonPlayerEntity>()
		.set<_InternalComponent_ComponentConstructorBasePointer>({nullptr});
	ecs.component<TagPlayerEntity>()
		.set<_InternalComponent_ComponentConstructorBasePointer>({nullptr});

	ecs.component<ComponentPlayerConnectionPeer>()
		.set<_InternalComponent_ComponentConstructorBasePointer>({nullptr});
	return 0;
}
