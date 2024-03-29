#pragma once

#include <string>

namespace ClientRpcFunctionNames
{
// void JoinRealm(std::string)
inline const std::string JointRealm = "JoinRealm";

// void SpawnEntities({entityId, EntityMovementState, EntityName, EntityModel,
// EntityShape, EntityMovementParams}, ...)
inline const std::string SpawnEntities = "SpawnEntities";

// void UpdateEntities({entityId, EntityMovementState}, ...)
inline const std::string UpdateEntities = "UpdateEntities";

// void SetModel(entityId, EntityModel, EntityShape)
inline const std::string SetModel = "SetModel";

// void DeleteEntities({entityId}, ...)
inline const std::string DeleteEntities = "DeleteEntities";

// void SetPlayerEntityId(uint64_t)
inline const std::string SetPlayerEntityId = "SetPlayerEntityId";

// void SetCurrentTick(uint64_t)
inline const std::string SetCurrentTick = "SetCurrentTick";

// void SetGravity(float)
inline const std::string SetGravity = "SetGravity";
} // namespace ClientRpcFunctionNames
