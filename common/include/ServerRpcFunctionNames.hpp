#pragma once

#include <string>

namespace ServerRpcFunctionNames
{
// bool Login(string, string)
inline const std::string Login = "Login";

// void UpdatePlayer(EntityLastAuthoritativeMovementState)
inline const std::string UpdatePlayer = "UpdatePlayer";

// void GetEntitiesData({uint64_t entityId}, ...)
inline const std::string GetEntitiesData = "GetEntitiesData";

// void Ping(uint64_t internal)
inline const std::string Ping = "Ping";
} // namespace ServerRpcFunctionNames
