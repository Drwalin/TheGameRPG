#include "../../thirdparty/flecs/distr/flecs.h"

#include "../../include/ComponentCallbackRegistry.hpp"
#include "../../include/EntityGameComponents.hpp"
#include "../../include/SharedObject.hpp"
#include "../../include/RealmServer.hpp"
#include "../../include/callbacks/CallbackOnUse.hpp"

inline float TransformDistance(const ComponentStaticTransform &l,
							   const ComponentStaticTransform &r)
{
	glm::vec3 a = l.trans.pos - r.trans.pos;
	float dr =
		abs(l.trans.rot.value - r.trans.rot.value) * M_PI * 2.0f / 240.0f;
	float c = fabs(l.scale - r.scale);
	return glm::dot(a, a) + dr + c;
}

void OnUse_SingleDoor(RealmServer *realm, uint64_t instigatorId,
					  uint64_t receiverId, const std::string &context)
{
	flecs::entity target = realm->Entity(receiverId);
	if (target.has<ComponentStaticTransform>()) {
		const ComponentSingleDoorTransformStates *states =
			target.try_get<ComponentSingleDoorTransformStates>();
		const ComponentStaticTransform *transform =
			target.try_get<ComponentStaticTransform>();

		if (TransformDistance(*transform, states->transformOpen) >
			TransformDistance(*transform, states->transformClosed)) {
			target.set<ComponentStaticTransform>(states->transformOpen);
		} else {
			target.set<ComponentStaticTransform>(states->transformClosed);
		}
	} else {
		LOG_WARN("Entity %lu does not have ComponentStaticTransform but had "
				 "called OnUse_SingleDoor",
				 receiverId);
	}
}

extern "C" void
Register_OnUse_SingleDoor(class ServerCore *serverCore,
						  std::shared_ptr<SharedObject> sharedObject)
{
	named_callbacks::registry_entries::OnUse::Set(
		"OpenableSingleDoor", "OpSD", &OnUse_SingleDoor, sharedObject);
}
