#pragma once

#include <string>

namespace ServerRpcFunctionNames
{
// void SetUsername(std::string)
inline const std::string SetUsername = "SetUsername";

// void UpdatePlayer(EntityLastAuthoritativeMovementState)
inline const std::string UpdatePlayer = "UpdatePlayer";

// void GetEntitiesData({uint64_t entityId}, ...)
inline const std::string GetEntitiesData = "GetEntitiesData";

// void GetCurrentTick(void)
inline const std::string GetCurrentTick = "GetCurrentTick";
} // namespace ServerRpcFunctionNames
