#pragma once

#include <flecs.h>

#include "../../common/include/ComponentCharacterSheet.hpp"

#include "EntityGameComponents.hpp"

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
void Spawner(RealmServer *realm, flecs::entity entity,
			 ComponentSpawner &spawner,
			 const ComponentStaticTransform &transform);
} // namespace GameLogic
