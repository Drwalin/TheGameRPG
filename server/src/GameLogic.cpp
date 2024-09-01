#include "../include/GameLogic.hpp"
#include "../include/RealmServer.hpp"

#define SYSTEM(FUNC) System(#FUNC, FUNC)

void RealmServer::RegisterGameLogic()
{
	System(GameLogic::LevelUp);
	System(GameLogic::HealthRegenerate);
}
