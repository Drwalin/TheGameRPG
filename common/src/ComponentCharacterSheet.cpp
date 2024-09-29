#include "../include/RegistryComponent.inl.hpp"

#include "../include/ComponentCharacterSheet.hpp"

int RegisterEntityComponentsCharacterSheet(flecs::world &ecs)
{
	REGISTER_COMPONENT_FOR_ECS_WORLD(ecs, ComponentCharacterSheet_Ranges,
									 "CS_RANGE");
	REGISTER_COMPONENT_FOR_ECS_WORLD(ecs, ComponentCharacterSheet_Health,
									 "CS_HP");
	REGISTER_COMPONENT_FOR_ECS_WORLD(ecs, ComponentCharacterSheet_HealthRegen,
									 "CS_HPREGEN");
	REGISTER_COMPONENT_FOR_ECS_WORLD(ecs, ComponentCharacterSheet_LevelXP,
									 "CS_XP");
	REGISTER_COMPONENT_FOR_ECS_WORLD(ecs, ComponentCharacterSheet_Strength,
									 "CS_STR");
	REGISTER_COMPONENT_FOR_ECS_WORLD(
		ecs, ComponentCharacterSheet_AttackCooldown, "CS_ATACK");
	REGISTER_COMPONENT_FOR_ECS_WORLD(ecs, ComponentCharacterSheet_Protection,
									 "CS_AP");

	reg::ComponentConstructor<ComponentCharacterSheet_HealthRegen>::singleton
		->callbackDeserializePersistent =
		[](class Realm *realm, flecs::entity entity,
		   ComponentCharacterSheet_HealthRegen *state) {
			state->lastTimestamp += realm->timer.currentTick;
		};
	reg::ComponentConstructor<ComponentCharacterSheet_HealthRegen>::singleton
		->callbackSerializePersistent =
		[](class Realm *realm, ComponentCharacterSheet_HealthRegen *state) {
			state->lastTimestamp -= realm->timer.currentTick;
		};

	reg::ComponentConstructor<ComponentCharacterSheet_AttackCooldown>::singleton
		->callbackDeserializePersistent =
		[](class Realm *realm, flecs::entity entity,
		   ComponentCharacterSheet_AttackCooldown *state) {
			state->lastTimestamp += realm->timer.currentTick;
		};
	reg::ComponentConstructor<ComponentCharacterSheet_AttackCooldown>::singleton
		->callbackSerializePersistent =
		[](class Realm *realm, ComponentCharacterSheet_AttackCooldown *state) {
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
