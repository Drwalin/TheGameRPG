#pragma once

#include <string>

namespace ClientRpcFunctionNames
{
// void SetRealms(std::vector<std::string>)
inline const std::string SetRealms = "SetRealms";

// void SpawnEntities({entityId, EntityMovementState, EntityLongState}, ...)
inline const std::string SpawnEntities = "SpawnEntities";

// void UpdateEntities({entityId, lastUpdateTick, vel, pos, forward}, ...)
inline const std::string UpdateEntities = "UpdateEntities";

// void SetModel(entityId, modelName, height, width)
inline const std::string SetModel = "SetModel";

// void DeleteEntities({entityId}, ...)
inline const std::string DeleteEntities = "DeleteEntities";

// void SetPlayerEntityId(uint64_t)
inline const std::string SetPlayerEntityId = "SetPlayerEntityId";

// void SetCurrentTick(uint64_t)
inline const std::string SetCurrentTick = "SetCurrentTick";

// void Pong(uint64_t)
inline const std::string Ping = "Ping";

// void SetGravity(float)
inline const std::string SetGravity = "SetGravity";
} // namespace ClientRemoteFunctions
