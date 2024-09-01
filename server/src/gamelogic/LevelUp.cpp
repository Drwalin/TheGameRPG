#include "../include/GameLogic.hpp"

namespace GameLogic
{
void LevelUp(flecs::entity entity, ComponentCharacterSheet_LevelXP lvl,
			 ComponentCharacterSheet_Health hp,
			 ComponentCharacterSheet_HealthRegen hpReg,
			 ComponentCharacterSheet_Strength str,
			 ComponentCharacterSheet_Protection ap)
{
	bool update = false;
	while (true) {
		auto lv = lvl.level;
		auto xpReq = (lv+1) * (lv) * 10 + 10;
		if (lvl.xp >= xpReq) {
			lvl.level += 1;
			lvl.xp -= xpReq;
			switch ((lvl.level-1) % 4) {
				case 0:
					hp.maxHP += 2;
					entity.set(hp);
					break;
				case 1:
					str.strength += 1;
					entity.set(str);
					break;
				case 2:
					ap.armorPoints += 1;
					entity.set(ap);
					break;
				case 3:
					hp.maxHP += 2;
					entity.set(hp);
					break;
			}
			update = true;
		} else {
			break;
		}
	}
	if (update) {
		entity.set(lvl);
	}
}
}
