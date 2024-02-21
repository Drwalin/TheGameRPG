#pragma once

#include <string>

#include "icon7/Flags.hpp"
#include "icon7/Peer.hpp"
#include "icon7/Host.hpp"
#include "icon7/RPCEnvironment.hpp"

namespace ServerRemoteFunctions
{
// void SetUsername(std::string)
inline const std::string SetUsername = "SetUsername";

// void UpdatePlayer(tick, pos, vel, forward)
inline const std::string UpdatePlayer = "UpdatePlayer";

// void GetRealms(void)
inline const std::string GetRealms = "GetRealms";

// void JoinRealm(std::string)
inline const std::string JoinRealm = "JoinRealm";

// void GetEntitiesData({uint64_t entityId}, ...)
inline const std::string GetEntitiesData = "GetEntitiesData";

// void GetTerrain(void)
inline const std::string GetTerrain = "GetTerrain";
} // namespace ServerRemoteFunctions
