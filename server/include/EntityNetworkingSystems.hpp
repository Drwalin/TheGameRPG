#pragma once

#include <flecs.h>

#include "../../common/include/EntityData.hpp"

#include "EntityDataServer.hpp"

class RealmServer;

namespace EntityNetworkingSystems
{
void OnPlayerEntityConnected(RealmServer *realm, flecs::entity entity,
							 const EntityPlayerConnectionPeer &peer);

void OnNewEntitySpawned(RealmServer *realm, flecs::entity entity,
						const EntityMovementState &state,
						const EntityShape &shape,
						const EntityModelName &entityModelName,
						const EntityName &entityName,
						const EntityMovementParameters &movementParams);

void OnPeerDisconnected(RealmServer *realm, flecs::entity entity,
						const EntityPlayerConnectionPeer &peer,
						const EntityName &entityName);

void RegisterObservers(RealmServer *realm);
}; // namespace EntityNetworkingSystems
