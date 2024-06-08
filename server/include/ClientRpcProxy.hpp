#pragma once

#include <icon7/RPCEnvironment.hpp>
#include <icon7/Peer.hpp>

#include "../../common/include/EntityData.hpp"

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
						const std::string &modelName, EntityShape shape);
void Broadcast_SpawnEntity(RealmServer *realm, uint64_t entityId,
						   const EntityMovementState &state,
						   const EntityShape &shape,
						   const EntityModelName &entityModelName,
						   const EntityName &entityName,
						   const EntityMovementParameters &movementParams);
void Broadcast_UpdateEntities(std::shared_ptr<RealmServer> realm);
void Broadcast_DeleteEntity(RealmServer *realm, uint64_t entityId);

void LoginSuccessfull(icon7::Peer *peer);
void LoginFailed(icon7::Peer *peer);

void Broadcast_SpawnStaticEntities(RealmServer *realm, uint64_t entityId,
								   const EntityStaticTransform &transform,
								   const EntityModelName &model,
								   const EntityStaticCollisionShapeName &shape);
void SpawnStaticEntities_ForPeer(RealmServer *realm, icon7::Peer *peer);
} // namespace ClientRpcProxy
