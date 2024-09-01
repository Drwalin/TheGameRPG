#include "../include/RegistryComponent.inl.hpp"

#include "../include/ComponentCharacterSheet.hpp"

GAME_REGISTER_ECS_COMPONENT_STATIC(ComponentCharacterSheet_Ranges, "CS_RANGE");

GAME_REGISTER_ECS_COMPONENT_STATIC(ComponentCharacterSheet_Health, "CS_HP");

GAME_REGISTER_ECS_COMPONENT_STATIC(ComponentCharacterSheet_HealthRegen,
								   "CS_HPREGEN");

GAME_REGISTER_ECS_COMPONENT_STATIC(ComponentCharacterSheet_LevelXP, "CS_XP");

GAME_REGISTER_ECS_COMPONENT_STATIC(ComponentCharacterSheet_Strength, "CS_STR");

GAME_REGISTER_ECS_COMPONENT_STATIC(ComponentCharacterSheet_AttackCooldown,
								   "CS_ATACK");

GAME_REGISTER_ECS_COMPONENT_STATIC(ComponentCharacterSheet_Protection, "CS_AP");

int RegisterEntityComponentsCharacterSheet(flecs::world &ecs)
{
	ecs.component<ComponentCharacterSheet_Ranges>();
	ecs.component<ComponentCharacterSheet_Health>();
	ecs.component<ComponentCharacterSheet_HealthRegen>();
	ecs.component<ComponentCharacterSheet_LevelXP>();
	ecs.component<ComponentCharacterSheet_Strength>();
	ecs.component<ComponentCharacterSheet_AttackCooldown>();
	ecs.component<ComponentCharacterSheet_Protection>();
	
	reg::ComponentConstructor<ComponentCharacterSheet_HealthRegen>::singleton
		->callbackDeserializePersistent = [](class Realm *realm,
											 flecs::entity entity,
											 ComponentCharacterSheet_HealthRegen *state) {
		state->lastTimestamp += realm->timer.currentTick;
	};
	reg::ComponentConstructor<ComponentCharacterSheet_HealthRegen>::singleton
		->callbackSerializePersistent = [](class Realm *realm,
										   ComponentCharacterSheet_HealthRegen *state) {
		state->lastTimestamp -= realm->timer.currentTick;
	};
	
	reg::ComponentConstructor<ComponentCharacterSheet_AttackCooldown>::singleton
		->callbackDeserializePersistent = [](class Realm *realm,
											 flecs::entity entity,
											 ComponentCharacterSheet_AttackCooldown *state) {
		state->lastTimestamp += realm->timer.currentTick;
	};
	reg::ComponentConstructor<ComponentCharacterSheet_AttackCooldown>::singleton
		->callbackSerializePersistent = [](class Realm *realm,
										   ComponentCharacterSheet_AttackCooldown *state) {
		state->lastTimestamp -= realm->timer.currentTick;
	};
	
	return 0;
}

BITSCPP_BYTESTREAM_OP_SYMMETRIC_DEFINITIONS(ComponentCharacterSheet_Ranges, {
	s.op(attackRange);
	s.op(visionRange);
});

BITSCPP_BYTESTREAM_OP_SYMMETRIC_DEFINITIONS(ComponentCharacterSheet_Health, {
	s.op(maxHP);
	s.op(hp);
});

BITSCPP_BYTESTREAM_OP_SYMMETRIC_DEFINITIONS(ComponentCharacterSheet_HealthRegen,
											{
												s.op(cooldown);
												s.op(amount);
												s.op(lastTimestamp);
											});

BITSCPP_BYTESTREAM_OP_SYMMETRIC_DEFINITIONS(ComponentCharacterSheet_LevelXP, {
	s.op(xp);
	s.op(level);
});
BITSCPP_BYTESTREAM_OP_SYMMETRIC_DEFINITIONS(ComponentCharacterSheet_Strength,
											{ s.op(strength); });

BITSCPP_BYTESTREAM_OP_SYMMETRIC_DEFINITIONS(
	ComponentCharacterSheet_AttackCooldown, {
		s.op(baseCooldown);
		s.op(lastTimestamp);
	});

BITSCPP_BYTESTREAM_OP_SYMMETRIC_DEFINITIONS(ComponentCharacterSheet_Protection,
											{ s.op(armorPoints); });
