#include "../../include/ClientRpcProxy.hpp"
#include "../../include/EntityNetworkingSystems.hpp"
#include "../../include/RealmServer.hpp"

#include "../../include/GameLogic.hpp"

namespace GameLogic
{
static void HealthRegenerate(RealmServer *realm, int64_t scheduledTick,
							 int64_t currentTick, uint64_t entityId);

static EntityEventTemplate eventHealthRegen{
	"HealthRegenerate", (EntityEventTemplate::CallbackType)HealthRegenerate};

static void HealthRegenerate(RealmServer *realm, int64_t scheduledTick,
							 int64_t currentTick, uint64_t entityId)
{
	flecs::entity entity = realm->Entity(entityId);
	const ComponentCharacterSheet_Health *_hp =
		entity.get<ComponentCharacterSheet_Health>();
	const ComponentCharacterSheet_HealthRegen *_hpReg =
		entity.get<ComponentCharacterSheet_HealthRegen>();
	if (_hp == nullptr || _hpReg == nullptr) {
		return;
	}
	auto hp = *_hp;
	auto hpReg = *_hpReg;

	if (hp.hp <= 0) {
		realm->ScheduleEntityEvent(
			entityId,
			{hpReg.lastTimestamp + hpReg.cooldown, &eventHealthRegen});
		return;
	}
	int64_t count =
		(realm->timer.currentTick - hpReg.lastTimestamp) / hpReg.cooldown;
	hpReg.lastTimestamp += count * hpReg.cooldown;
	if (hp.hp >= hp.maxHP) {
		realm->ScheduleEntityEvent(
			entityId,
			{hpReg.lastTimestamp + hpReg.cooldown, &eventHealthRegen});
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
	realm->ScheduleEntityEvent(
		entityId, {hpReg.lastTimestamp + hpReg.cooldown, &eventHealthRegen});
}

void HealthRegenerateSchedule(flecs::entity entity,
							  ComponentCharacterSheet_Health hp,
							  ComponentCharacterSheet_HealthRegen hpReg,
							  ComponentEventsQueue &)
{
	if (hp.hp <= 0) {
		return;
	}
	RealmPtr *rp = entity.world().get_mut<RealmPtr>();
	rp->realm->ScheduleEntityEvent(
		entity, {hpReg.lastTimestamp + hpReg.cooldown, &eventHealthRegen});
}
} // namespace GameLogic
