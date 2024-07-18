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

void InteractInLineOfSight(GameClient *gameClient, ComponentMovementState state,
						   uint64_t targetId, glm::vec3 dstPos,
						   glm::vec3 normal);

void Attack(GameClient *gameClient, ComponentMovementState state,
			uint64_t targetId, glm::vec3 targetPos, int64_t attackType,
			int64_t attackId, const std::string &argStr, int64_t argInt);
} // namespace ServerRpcProxy
