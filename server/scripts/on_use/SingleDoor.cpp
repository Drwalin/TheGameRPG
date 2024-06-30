#include <flecs.h>

#include "../../include/ComponentCallbackRegistry.hpp"
#include "../../include/EntityGameComponents.hpp"
#include "../../include/SharedObject.hpp"
#include "../../include/RealmServer.hpp"

inline float TransformDistance(const ComponentStaticTransform &l,
							   const ComponentStaticTransform &r)
{
	glm::vec3 a = l.pos - r.pos;
	glm::quat b = l.rot - r.rot;
	glm::vec3 c = l.scale - r.scale;
	return glm::dot(a, a) + glm::dot(b, b) + glm::dot(c, c);
}

void OnUse_SingleDoor(RealmServer *realm, uint64_t instigatorId,
					  uint64_t receiverId, const std::string &context)
{
	flecs::entity target = realm->Entity(receiverId);
	if (target.has<ComponentStaticTransform>()) {
		const ComponentSingleDoorTransformStates *states =
			target.get<ComponentSingleDoorTransformStates>();
		const ComponentStaticTransform *transform =
			target.get<ComponentStaticTransform>();

		if (TransformDistance(*transform, states->transformOpen)
				> TransformDistance(*transform, states->transformClosed)) {
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
	REGISTER_NAMED_CALLBACK(named_callbacks::registry_entries::OnUse,
							"OpenableSingleDoor", "OpSD", &OnUse_SingleDoor,
							sharedObject);
}
