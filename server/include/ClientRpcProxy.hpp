#pragma once

#include <icon7/RPCEnvironment.hpp>
#include <icon7/Peer.hpp>

#include "../../common/include/EntityComponents.hpp"

class RealmServer;

namespace ClientRpcProxy
{
void JoinRealm(RealmServer *realm, icon7::Peer *peer);

void SetPlayerEntityId(RealmServer *realm, icon7::Peer *peer,
					   uint64_t playerEntityId);
void Pong(icon7::Peer *peer, icon7::Flags flags, int64_t data);
void SetGravity(RealmServer *realm, icon7::Peer *peer, float gravity);

void DeleteEntity_ForPeer(std::shared_ptr<RealmServer> realm, icon7::Peer *peer,
						  uint64_t entityId);
void SpawnEntities_ForPeer(std::shared_ptr<RealmServer> realm,
						   icon7::Peer *peer);

void SpawnEntities_ForPeerByIds(std::shared_ptr<RealmServer> realm,
								icon7::Peer *peer, icon7::ByteReader &reader);

void Broadcast_SetModel(std::shared_ptr<RealmServer> realm, uint64_t entityId,
						const std::string &modelName, ComponentShape shape);
void Broadcast_SpawnEntity(RealmServer *realm, uint64_t entityId,
						   const ComponentMovementState &state,
						   const ComponentShape &shape,
						   const ComponentModelName &entityModelName,
						   const ComponentName &entityName,
						   const ComponentMovementParameters &movementParams);
void Broadcast_UpdateEntities(std::shared_ptr<RealmServer> realm);
void Broadcast_DeleteEntity(RealmServer *realm, uint64_t entityId);

void LoginSuccessfull(icon7::Peer *peer);
void LoginFailed(icon7::Peer *peer);

void Broadcast_SpawnStaticEntities(
	RealmServer *realm, uint64_t entityId,
	const ComponentStaticTransform &transform, const ComponentModelName &model,
	const ComponentStaticCollisionShapeName &shape);
void SpawnStaticEntities_ForPeer(RealmServer *realm, icon7::Peer *peer);
} // namespace ClientRpcProxy
