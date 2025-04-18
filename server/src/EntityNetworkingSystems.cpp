#include <icon7/Peer.hpp>
#include <icon7/Debug.hpp>

#include "../include/RealmServer.hpp"

#include "../include/ClientRpcProxy.hpp"
#include "../include/RealmServer.hpp"
#include "../include/EntityNetworkingSystems.hpp"

namespace EntityNetworkingSystems
{
void OnPlayerEntityConnected(RealmServer *realm, flecs::entity entity,
							 const ComponentPlayerConnectionPeer &peer)
{
	ClientRpcProxy::SetGravity(realm, peer.peer.get(), realm->gravity);
	ClientRpcProxy::JoinRealm(realm, peer.peer.get(), entity.id());
}

void OnNewEntitySpawned(RealmServer *realm, flecs::entity entity,
						const ComponentLastAuthoritativeMovementState &state,
						const ComponentShape &shape,
						const ComponentModelName &entityModelName,
						const ComponentName &entityName,
						const ComponentMovementParameters &movementParams)
{
	ClientRpcProxy::Broadcast_SpawnEntity(realm, entity, state, shape,
										  entityModelName, entityName,
										  movementParams);
}

void OnPeerDisconnected(RealmServer *realm, flecs::entity entity,
						const ComponentPlayerConnectionPeer &peer,
						const ComponentName &entityName)
{
	if (peer.peer.get() == nullptr) {
		return;
	}

	// Probably duplicate of some of RealmServer::DisconnectPeer code.

	// TODO: check if this realm->peers.erase() is required or if it's in
	//       observer
	realm->peers.erase(peer.peer);
	PeerData *data = ((PeerData *)(peer.peer->userPointer));
	data->realm.reset();
	data->entityId = 0;
}

void RegisterObservers(RealmServer *realm)
{
	realm->RegisterObserver(flecs::OnSet,
							[realm](flecs::entity entity,
									const ComponentPlayerConnectionPeer &peer) {
								OnPlayerEntityConnected(realm, entity, peer);
							});

	realm->RegisterObserver(
		flecs::OnAdd,
		[realm](flecs::entity entity,
				const ComponentLastAuthoritativeMovementState &state,
				const ComponentShape &shape,
				const ComponentModelName &entityModelName,
				const ComponentName &entityName,
				const ComponentMovementParameters &movementParams) {
			OnNewEntitySpawned(realm, entity, state, shape, entityModelName,
							   entityName, movementParams);
		});

	realm->RegisterObserver(
		flecs::OnRemove,
		[realm](flecs::entity entity, ComponentPlayerConnectionPeer &peer,
				const ComponentName &entityName) {
			OnPeerDisconnected(realm, entity, peer, entityName);
		});
}
} // namespace EntityNetworkingSystems
