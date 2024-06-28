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

// void Ping(int64_t internal)
inline const std::string Ping = "Ping";

// void InteractInLineOfSight(uint64_t targetId, uint64_t timestamp,
//                            vec3 srcPos, vec3 dstPos, vec3 normal)
inline const std::string InteractInLineOfSight = "InteractInLineOfSight";
} // namespace ServerRpcFunctionNames
