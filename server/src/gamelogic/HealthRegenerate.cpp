#include "../../include/ClientRpcProxy.hpp"
#include "../../include/EntityNetworkingSystems.hpp"
#include "../../include/RealmServer.hpp"

#include "../../include/GameLogic.hpp"

namespace GameLogic
{
static int64_t HealthRegenerate(RealmServer *realm, int64_t scheduledTick,
								int64_t currentTick, uint64_t entityId);

static EntityEventTemplate eventHealthRegen{
	"HealthRegenerate", (EntityEventTemplate::CallbackType)HealthRegenerate};

static int64_t HealthRegenerate(RealmServer *realm, int64_t scheduledTick,
								int64_t currentTick, uint64_t entityId)
{
	flecs::entity entity = realm->Entity(entityId);
	const ComponentCharacterSheet_Health *_hp =
		entity.try_get<ComponentCharacterSheet_Health>();
	const ComponentCharacterSheet_HealthRegen *_hpReg =
		entity.try_get<ComponentCharacterSheet_HealthRegen>();
	if (_hp == nullptr || _hpReg == nullptr) {
		return 0;
	}
	auto hp = *_hp;
	auto hpReg = *_hpReg;

	if (hp.hp <= 0) {
		return hpReg.cooldown;
	}
	int64_t count =
		(realm->timer.currentTick - hpReg.lastTimestamp) / hpReg.cooldown;
	hpReg.lastTimestamp += count * hpReg.cooldown;
	if (hp.hp >= hp.maxHP) {
		return std::max<int64_t>(10, hpReg.lastTimestamp + hpReg.cooldown -
										 currentTick);
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
	return std::max<int64_t>(10, hpReg.lastTimestamp + hpReg.cooldown -
									 currentTick);
}

void HealthRegenerateSchedule(flecs::entity entity,
							  ComponentCharacterSheet_Health hp,
							  ComponentCharacterSheet_HealthRegen hpReg,
							  ComponentEventsQueue &)
{
	if (hp.hp <= 0) {
		return;
	}
	RealmPtr *rp = entity.world().try_get_mut<RealmPtr>();
	rp->realm->ScheduleEntityEvent(
		entity, {hpReg.lastTimestamp + hpReg.cooldown, &eventHealthRegen});
}
} // namespace GameLogic
