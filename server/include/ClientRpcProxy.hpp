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

void DeleteEntity_ForPeer(RealmServer *realm, icon7::Peer *peer,
						  uint64_t entityId);
void SpawnEntities_ForPeer(RealmServer *realm, icon7::Peer *peer);

void SpawnEntities_ForPeerByIds(RealmServer *realm, icon7::Peer *peer,
								icon7::ByteReader &reader);

void Broadcast_SetModel(RealmServer *realm, uint64_t entityId,
						const std::string &modelName, EntityShape shape);
void Broadcast_SpawnEntity(RealmServer *realm, uint64_t entityId,
						   const EntityMovementState &state,
						   const EntityShape &shape,
						   const EntityModelName &entityModelName,
						   const EntityName &entityName);
void Broadcast_UpdateEntities(RealmServer *realm);
void Broadcast_DeleteEntity(RealmServer *realm, uint64_t entityId);

void LoginSuccessfull(icon7::Peer *peer);
void LoginFailed(icon7::Peer *peer);
} // namespace ClientRpcProxy
