#pragma once

#include <string>

namespace ServerRpcFunctionNames
{
// bool Login(string userName)
inline const std::string Login = "Login";

// void UpdatePlayer(EntityLastAuthoritativeMovementState)
inline const std::string UpdatePlayer = "UpdatePlayer";

// void GetEntitiesData({uint64_t entityId}, ...)
inline const std::string GetEntitiesData = "GetEntitiesData";

// void Ping(int64_t internal, int64_t internal, int64_t currentTimestamp)
inline const std::string Ping = "Ping";

// void InteractInLineOfSight(ComponentMovementState, uint64_t targetId,
// 							  vec3 dstPos, vec3 normal)
inline const std::string InteractInLineOfSight = "InteractInLineOfSight";

// void Attack(ComponentMovementState, uint64_t targetId, uint64_t targetPos,
// 			   string attackName, int64_t attackId, int64_t argInt)
inline const std::string Attack = "Attack";
} // namespace ServerRpcFunctionNames
