#pragma once

#include <icon7/RPCEnvironment.hpp>
#include <icon7/Peer.hpp>

#include "../../common/include/EntityData.hpp"
#include "EntityDataServer.hpp"

namespace ClientRpcProxy
{
void SetPlayerEntityId(RealmServer *realm, icon7::Peer *peer,
					   uint64_t playerEntityId);
void SetCurrentTick(RealmServer *realm, icon7::Peer *peer);
void Pong(icon7::Peer *peer, uint64_t data);
void SetGravity(RealmServer *realm, icon7::Peer *peer, float gravity);

void DeleteEntity_ForPeer(RealmServer *realm, icon7::Peer *peer,
						  uint64_t entityId);
void SpawnEntities_ForPeer(RealmServer *realm, icon7::Peer *peer);

void SpawnEntities_ForPeerByIds(RealmServer *realm, icon7::Peer *peer,
		icon7::ByteReader &reader);

void Broadcast_SetModel(RealmServer *realm, uint64_t entityId,
						const std::string &modelName, EntityShape shape);
void Broadcast_SpawnEntity(RealmServer *realm, flecs::entity entity,
						   const EntityMovementState &state,
						   const EntityShape &shape,
						   const EntityModelName &entityModelName,
						   const EntityName &entityName);
void Broadcast_UpdateEntities(RealmServer *realm);
void Broadcast_DeleteEntity(RealmServer *realm, uint64_t entityId);
} // namespace ClientRpcProxy
