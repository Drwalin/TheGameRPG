#pragma once

#include "../../common/include/ComponentCharacterSheet.hpp"

#include <flecs.h>

class Realm;
class RealmServer;

namespace GameLogic
{
void LevelUp(flecs::entity entity, ComponentCharacterSheet_LevelXP lvl,
			 ComponentCharacterSheet_Health hp,
			 ComponentCharacterSheet_HealthRegen hpReg,
			 ComponentCharacterSheet_Strength str,
			 ComponentCharacterSheet_Protection ap);
void HealthRegenerate(RealmServer *realm, flecs::entity entity,
					  ComponentCharacterSheet_Health hp,
					  ComponentCharacterSheet_HealthRegen hpReg);
} // namespace GameLogic
