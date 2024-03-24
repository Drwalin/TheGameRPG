#pragma once

#include "../../common/include/EntityData.hpp"

#include "EntityDataServer.hpp"

class RealmServer;

namespace EntityNetworkingSystems
{
void OnPlayerEntityConnected(RealmServer *realm, flecs::entity entity,
							 EntityPlayerConnectionPeer &peer,
							 const EntityName &entityName);

void OnPeerDisconnected(RealmServer *realm, flecs::entity entity,
						EntityPlayerConnectionPeer &peer,
						const EntityName &entityName);

void InitObservers(RealmServer *realm);
}; // namespace EntityNetworkingSystems
