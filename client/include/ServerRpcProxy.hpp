#pragma once

#include "../../common/include/EntityComponents.hpp"

class GameClient;

namespace ServerRpcProxy
{
void Login(GameClient *gameClient, std::string username, std::string password);

void UpdatePlayer(GameClient *gameClient, const ComponentMovementState &state);

void GetEntitiesData(GameClient *gameClient,
					 const std::vector<uint64_t> &entities);

void Ping(GameClient *gameClient, bool reliable);

void InteractInLineOfSight(GameClient *gameClient, uint64_t targetId,
						   glm::vec3 srcPos, glm::vec3 dstPos,
						   glm::vec3 normal);
} // namespace ServerRpcProxy
