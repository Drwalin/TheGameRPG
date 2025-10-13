#pragma once

#include "ComponentsUtility.hpp"
#include "Tick.hpp"

struct ComponentCharacterSheet_Ranges {
	float attackRange = 4.0f;
	float visionRange = 20.0f;

	ComponentCharacterSheet_Ranges(float attackRange, float visionRange)
		: attackRange(attackRange), visionRange(visionRange)
	{
	}

	BITSCPP_BYTESTREAM_OP_DECLARATIONS()
	DEFAULT_CONSTRUCTORS_AND_MOVE(ComponentCharacterSheet_Ranges,
								  {MV(attackRange) MV(visionRange)});
};

struct ComponentCharacterSheet_Health {
	int32_t maxHP = 10;
	int32_t hp = 10;

	ComponentCharacterSheet_Health(int32_t maxHP, int32_t hp)
		: maxHP(maxHP), hp(hp)
	{
	}

	BITSCPP_BYTESTREAM_OP_DECLARATIONS()
	DEFAULT_CONSTRUCTORS_AND_MOVE(ComponentCharacterSheet_Health,
								  {MV(maxHP) MV(hp)});
};

struct ComponentCharacterSheet_HealthRegen {
	int32_t cooldown = 60000;
	int32_t amount = 1;
	Tick lastTimestamp = {0};

	ComponentCharacterSheet_HealthRegen(int32_t cooldown, int32_t amount,
										Tick lastTimestamp)
		: cooldown(cooldown), amount(amount), lastTimestamp(lastTimestamp)
	{
	}

	BITSCPP_BYTESTREAM_OP_DECLARATIONS()
	DEFAULT_CONSTRUCTORS_AND_MOVE(ComponentCharacterSheet_HealthRegen,
								  {MV(cooldown) MV(amount) MV(lastTimestamp)});
};

struct ComponentCharacterSheet_LevelXP {
	int32_t level = 0;
	int64_t xp = 0;

	ComponentCharacterSheet_LevelXP(int32_t level, int64_t xp)
		: level(level), xp(xp)
	{
	}

	BITSCPP_BYTESTREAM_OP_DECLARATIONS()
	DEFAULT_CONSTRUCTORS_AND_MOVE(ComponentCharacterSheet_LevelXP,
								  {MV(xp) MV(level)});
};

struct ComponentCharacterSheet_Strength {
	int32_t strength = 0;

	ComponentCharacterSheet_Strength(int32_t strength) : strength(strength) {}

	BITSCPP_BYTESTREAM_OP_DECLARATIONS()
	DEFAULT_CONSTRUCTORS_AND_MOVE(ComponentCharacterSheet_Strength,
								  {MV(strength)});
};

struct ComponentCharacterSheet_AttackCooldown {
	int32_t baseCooldown = 500;
	Tick lastTimestamp = {0};

	ComponentCharacterSheet_AttackCooldown(int32_t baseCooldown,
										   Tick lastTimestamp)
		: baseCooldown(baseCooldown), lastTimestamp(lastTimestamp)
	{
	}

	BITSCPP_BYTESTREAM_OP_DECLARATIONS()
	DEFAULT_CONSTRUCTORS_AND_MOVE(ComponentCharacterSheet_AttackCooldown,
								  {MV(baseCooldown) MV(lastTimestamp)});
};

struct ComponentCharacterSheet_Protection {
	int32_t armorPoints = 0;

	ComponentCharacterSheet_Protection(int32_t armorPoints)
		: armorPoints(armorPoints)
	{
	}

	BITSCPP_BYTESTREAM_OP_DECLARATIONS()
	DEFAULT_CONSTRUCTORS_AND_MOVE(ComponentCharacterSheet_Protection,
								  {MV(armorPoints)});
};
