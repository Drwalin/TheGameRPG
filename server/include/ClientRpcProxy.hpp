#pragma once

#include <icon7/RPCEnvironment.hpp>
#include <icon7/Peer.hpp>

#include "../../common/include/EntityData.hpp"
#include "EntityDataServer.hpp"

namespace ClientRpcProxy
{
// void SpawnEntities({entityId, EntityMovementState, EntityLongState}, ...)
inline const std::string SpawnEntities = "SpawnEntities";

// void UpdateEntities({entityId, lastUpdateTick, vel, pos, forward}, ...)
inline const std::string UpdateEntities = "UpdateEntities";

// void DeleteEntities({entityId}, ...)
inline const std::string DeleteEntities = "DeleteEntities";



inline const std::string _SetRealms = "SetRealms";
void SetRealms(RealmServer *realm, icon7::Peer *peer,
		const std::vector<std::string> &realmNames);

inline const std::string _SetModel = "SetModel";
void SetModel(RealmServer *realm, icon7::Peer *peer, uint64_t entityId,
		const std::string &modelName, EntityShape shape);

inline const std::string _SetPlayerEntityId = "SetPlayerEntityId";
void SetPlayerEntityId(RealmServer *realm, icon7::Peer *peer,
		uint64_t playerEntityId);

inline const std::string _SetCurrentTick = "SetCurrentTick";
void SetCurrentTick(RealmServer *realm, icon7::Peer *peer,
		uint64_t playerEntityId);

inline const std::string _Ping = "Ping";
void Ping(RealmServer *realm, icon7::Peer *peer,
		uint64_t playerEntityId);

inline const std::string _SetGravity = "SetGravity";
void SetGravity(RealmServer *realm, icon7::Peer *peer,
		uint64_t playerEntityId);
}
