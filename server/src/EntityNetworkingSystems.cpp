#include "../include/RealmServer.hpp"

#include "../include/ClientRpcProxy.hpp"
#include "../include/RealmServer.hpp"
#include "../include/EntityNetworkingSystems.hpp"

namespace EntityNetworkingSystems
{
void OnPlayerEntityConnected(RealmServer *realm, flecs::entity entity,
							 const EntityPlayerConnectionPeer &peer,
							 const EntityName &entityName)
{
	ClientRpcProxy::SetPlayerEntityId(realm, peer.peer, entity.id());
}

void OnNewEntitySpawned(RealmServer *realm, flecs::entity entity,
						const EntityMovementState &state,
						const EntityShape &shape,
						const EntityModelName &entityModelName,
						const EntityName &entityName)
{
	ClientRpcProxy::Broadcast_SpawnEntity(realm, entity, state, shape,
										  entityModelName, entityName);
}

void OnPeerDisconnected(RealmServer *realm, flecs::entity entity,
						const EntityPlayerConnectionPeer &peer,
						const EntityName &entityName)
{
	ClientRpcProxy::Broadcast_DeleteEntity(realm, entity.id());
	realm->peers.erase(peer.peer);
}

void RegisterObservers(RealmServer *realm)
{
	realm->RegisterObserver(
		flecs::OnAdd,
		[realm](flecs::entity entity, const EntityPlayerConnectionPeer &peer,
				const EntityName &name) {
			OnPlayerEntityConnected(realm, entity, peer, name);
		});

	realm->RegisterObserver(flecs::OnAdd,
							[realm](flecs::entity entity,
									const EntityMovementState &state,
									const EntityShape &shape,
									const EntityModelName &entityModelName,
									const EntityName &entityName) {
								OnNewEntitySpawned(realm, entity, state, shape,
												   entityModelName, entityName);
							});

	realm->RegisterObserver(
		flecs::OnRemove,
		[realm](flecs::entity entity, EntityPlayerConnectionPeer &peer,
				const EntityName &entityName) {
			OnPeerDisconnected(realm, entity, peer, entityName);
		});
}

void RegisterSystems(RealmServer *realm)
{
}
} // namespace EntityNetworkingSystems
