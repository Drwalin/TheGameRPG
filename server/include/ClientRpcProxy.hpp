#pragma once

#include <icon7/RPCEnvironment.hpp>
#include <icon7/Peer.hpp>

#include "../../common/include/EntityData.hpp"
#include "EntityDataServer.hpp"

namespace ClientRpcProxy
{
void SetRealms(RealmServer *realm, icon7::Peer *peer,
			   const std::vector<std::string> &realmNames);
void SetPlayerEntityId(RealmServer *realm, icon7::Peer *peer,
					   uint64_t playerEntityId);
void SetCurrentTick(RealmServer *realm, icon7::Peer *peer,
					uint64_t playerEntityId);
void Ping(RealmServer *realm, icon7::Peer *peer);
void SetGravity(RealmServer *realm, icon7::Peer *peer, float gravity);

void DeleteEntity_ForPeer(RealmServer *realm, icon7::Peer *peer,
						  uint64_t entityId);
void SpawnEntities_ForPeer(RealmServer *realm, icon7::Peer *peer);

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
