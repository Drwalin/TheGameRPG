#include "../../thirdparty/flecs/distr/flecs.h"

#include "../../ICon7/include/icon7/Debug.hpp"
#include "../../ICon7/include/icon7/Peer.hpp"

#include "../../include/ComponentCallbackRegistry.hpp"
#include "../../include/EntityGameComponents.hpp"
#include "../../include/EntityComponentsServer.hpp"
#include "../../include/SharedObject.hpp"
#include "../../include/RealmServer.hpp"
#include "../../include/ServerCore.hpp"
#include "../../include/callbacks/CallbackOnTriggerEnterExit.hpp"

void OnTrigger_TeleportPlayer(RealmServer *realm, uint64_t entityId,
							  uint64_t triggerId)
{
	flecs::entity entity = realm->Entity(entityId);
	flecs::entity trigger = realm->Entity(triggerId);

	if (trigger.has<ComponentTeleport>()) {
		if (entity.has<ComponentPlayerConnectionPeer>()) {
			const ComponentPlayerConnectionPeer *_peer =
				entity.try_get<ComponentPlayerConnectionPeer>();
			const ComponentTeleport *tp = trigger.try_get<ComponentTeleport>();
			auto peer = _peer->peer;

			PeerData *data = ((PeerData *)(peer->userPointer));
			if (data) {
				data->nextRealm = tp->realmName;
				data->nextRealmPosition = tp->position;
				data->useNextRealmPosition = true;
				auto p = tp->position;
				LOG_INFO("Teleport %s into %s -> {%.1f %.1f %.1f}",
						 data->userName.c_str(), tp->realmName.c_str(), p.x,
						 p.y, p.z);
				realm->serverCore->ConnectPeerToRealm(peer.get());
			} else {
				LOG_ERROR("Peer 0x%p does not have initialised userData",
						  peer.get());
			}
		}
	} else {
		LOG_ERROR("Teleport trigger entity %lu does not have ComponentTeleport",
				  triggerId);
	}
}

extern "C" void
Register_OnTrigger_Teleport(class ServerCore *serverCore,
							std::shared_ptr<SharedObject> sharedObject)
{
	named_callbacks::registry_entries::OnTriggerEnterExit::Set(
		"TriggerTeleportPlayer", "TTPPL", &OnTrigger_TeleportPlayer,
		sharedObject);
}
