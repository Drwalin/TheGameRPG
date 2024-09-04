#include "../include/ClientRpcProxy.hpp"
#include "../include/EntityNetworkingSystems.hpp"
#include "../include/RealmServer.hpp"

#include "../include/GameLogic.hpp"

namespace GameLogic
{
void HealthRegenerate(RealmServer *realm, flecs::entity entity,
					  ComponentCharacterSheet_Health hp,
					  ComponentCharacterSheet_HealthRegen hpReg)
{
	if (hp.hp <= 0) {
		return;
	}
	int64_t count =
		(realm->timer.currentTick - hpReg.lastTimestamp) / hpReg.cooldown;
	hpReg.lastTimestamp += count * hpReg.cooldown;
	if (hp.hp >= hp.maxHP) {
		return;
	}
	if (count != 0) {
		entity.set<ComponentCharacterSheet_HealthRegen>(hpReg);
	}
	if (count > 0) {
		int64_t amount = count * hpReg.amount;
		if (hp.hp + amount > hp.maxHP) {
			hp.hp = hp.maxHP;
		} else {
			hp.hp += amount;
		}
		entity.set<ComponentCharacterSheet_Health>(hp);
	}
}
} // namespace GameLogic
