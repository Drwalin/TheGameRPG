#pragma once

#include <icon7/RPCEnvironment.hpp>
#include <icon7/Peer.hpp>

#include "../../common/include/EntityData.hpp"

class GameClient;

namespace ServerRpcProxy
{
void SetUsername(GameClient *gameClient, std::string username);

void UpdatePlayer(GameClient *gameClient, EntityLastAuthoritativeMovementState &state);

void GetEntitiesData(GameClient *gameClient, const std::vector<uint64_t> &entities);

void GetCurrentTick(GameClient *gameClient);

void Ping(GameClient *gameClient, uint64_t data);
}
