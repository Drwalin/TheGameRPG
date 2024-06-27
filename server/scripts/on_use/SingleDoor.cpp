
#include <flecs.h>

#include "../include/ComponentCallbackRegistry.hpp"
#include "../../include/EntityGameComponents.hpp"

#include "../include/RealmServer.hpp"

extern "C" void OnUse_SingleDoor(RealmServer *realm, uint64_t instigatorId,
								 uint64_t receiverId,
								 const std::string &context)
{
	flecs::entity target = realm->Entity(receiverId);
	if (target.has<ComponentStaticTransform>()) {
		const ComponentOpenableState *open = target.get<ComponentOpenableState>();
		const ComponentSingleDoorTransformStates *states = target.get<ComponentSingleDoorTransformStates>();
		
		bool op = !open->open;
		
		target.set<ComponentOpenableState>(ComponentOpenableState{op});
		if (op) {
			target.set<ComponentStaticTransform>(states->transformOpen);
		} else {
			target.set<ComponentStaticTransform>(states->transformClosed);
		}
	} else {
		LOG_WARN("Entity %lu does not have ComponentStaticTransform but had called OnUse_SingleDoor", receiverId);
	}
}

extern "C" void Register_OnUse_SingleDoor()
{
	REGISTER_NAMED_CALLBACK(named_callbacks::registry_entries::OnUse,
							"SingleDoor", "SD", OnUse_SingleDoor);
}
