#pragma once

#include "../../thirdparty/flecs/distr/flecs.h"

#include "../../common/include/EntityComponents.hpp"

#include "EntityComponentsServer.hpp"

class RealmServer;

namespace EntityNetworkingSystems
{
void OnPlayerEntityConnected(RealmServer *realm, flecs::entity entity,
							 const ComponentPlayerConnectionPeer &peer);

void OnNewEntitySpawned(RealmServer *realm, flecs::entity entity,
						const ComponentMovementState &state,
						const ComponentShape &shape,
						const ComponentModelName &entityModelName,
						const ComponentName &entityName,
						const ComponentMovementParameters &movementParams);

void OnPeerDisconnected(RealmServer *realm, flecs::entity entity,
						const ComponentPlayerConnectionPeer &peer,
						const ComponentName &entityName);

void RegisterObservers(RealmServer *realm);
}; // namespace EntityNetworkingSystems
