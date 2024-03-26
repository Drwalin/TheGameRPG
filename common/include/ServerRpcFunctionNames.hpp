#pragma once

#include <string>

namespace ServerRpcFunctionNames
{
// void SetUsername(std::string)
inline const std::string SetUsername = "SetUsername";

// void UpdatePlayer(EntityLastAuthoritativeMovementState)
inline const std::string UpdatePlayer = "UpdatePlayer";

// void UpdatePlayerRot(forward)
inline const std::string UpdatePlayerRot = "UpdatePlayerRot";

// void GetRealms(void)
inline const std::string GetRealms = "GetRealms";

// void JoinRealm(std::string)
inline const std::string JoinRealm = "JoinRealm";

// void GetEntitiesData({uint64_t entityId}, ...)
inline const std::string GetEntitiesData = "GetEntitiesData";

// void GetCurrentTick(void)
inline const std::string GetCurrentTick = "GetCurrentTick";

// void Ping(uint64_t)
inline const std::string Ping = "Ping";
} // namespace ServerRpcFunctionNames
