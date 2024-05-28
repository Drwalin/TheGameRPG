#pragma once

#include "../../common/include/EntityData.hpp"

class GameClient;

namespace ServerRpcProxy
{
void Login(GameClient *gameClient, std::string username, std::string password);

void UpdatePlayer(GameClient *gameClient, const EntityMovementState &state);

void GetEntitiesData(GameClient *gameClient,
					 const std::vector<uint64_t> &entities);

void Ping(GameClient *gameClient, bool reliable);
} // namespace ServerRpcProxy
