#include "../include/RealmServer.hpp"

#include "../include/ClientRpcProxy.hpp"
#include "../include/RealmServer.hpp"
#include "../include/EntityNetworkingSystems.hpp"

namespace EntityNetworkingSystems
{
void OnPlayerEntityConnected(RealmServer *realm, flecs::entity entity,
							 const EntityPlayerConnectionPeer &peer)
{
	DEBUG("Created entity");
	ClientRpcProxy::SetPlayerEntityId(realm, peer.peer.get(), entity.id());
	ClientRpcProxy::SetGravity(realm, peer.peer.get(), realm->gravity);
	ClientRpcProxy::JoinRealm(realm, peer.peer.get());
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
	realm->peers.erase(peer.peer.get());
	PeerData *data = ((PeerData *)(peer.peer->userPointer));
	data->realm = nullptr;
	data->entityId = 0;
}

void RegisterObservers(RealmServer *realm)
{
	realm->RegisterObserver(
		flecs::OnSet,
		[realm](flecs::entity entity, const EntityPlayerConnectionPeer &peer) {
			OnPlayerEntityConnected(realm, entity, peer);
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

void RegisterSystems(RealmServer *realm) {}
} // namespace EntityNetworkingSystems
