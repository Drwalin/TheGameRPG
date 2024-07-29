#pragma once

#include "ComponentsUtility.hpp"

struct ComponentCharacterSheet_Ranges {
	float attackRange = 4.0f;
	float visionRange = 20.0f;

	BITSCPP_BYTESTREAM_OP_DECLARATIONS();
	DEFAULT_CONSTRUCTORS_AND_MOVE(ComponentCharacterSheet_Ranges,
								  {MV(attackRange) MV(visionRange)});
};

struct ComponentCharacterSheet_Health {
	int32_t maxHP = 10;
	int32_t hp = 10;

	BITSCPP_BYTESTREAM_OP_DECLARATIONS();
	DEFAULT_CONSTRUCTORS_AND_MOVE(ComponentCharacterSheet_Health,
								  {MV(maxHP) MV(hp)});
};

struct ComponentCharacterSheet_HealthRegen {
	int32_t cooldown = 60000;
	int32_t amount = 1;
	int64_t lastTimestamp = 0;

	BITSCPP_BYTESTREAM_OP_DECLARATIONS();
	DEFAULT_CONSTRUCTORS_AND_MOVE(ComponentCharacterSheet_HealthRegen,
								  {MV(cooldown) MV(amount) MV(lastTimestamp)});
};

struct ComponentCharacterSheet_LevelXP {
	int64_t xpToNextLevel = 10;
	int64_t xp = 0;
	int32_t level = 0;
	int32_t xpDrop = 1;

	BITSCPP_BYTESTREAM_OP_DECLARATIONS();
	DEFAULT_CONSTRUCTORS_AND_MOVE(ComponentCharacterSheet_LevelXP,
								  {MV(xpToNextLevel) MV(xp) MV(level)
									   MV(xpDrop)});
};

struct ComponentCharacterSheet_Strength {
	int32_t strength = 0;

	BITSCPP_BYTESTREAM_OP_DECLARATIONS();
	DEFAULT_CONSTRUCTORS_AND_MOVE(ComponentCharacterSheet_Strength,
								  {MV(strength)});
};

struct ComponentCharacterSheet_AttackCooldown {
	int32_t baseCooldown = 500;
	int64_t lastTimestamp = 0;

	BITSCPP_BYTESTREAM_OP_DECLARATIONS();
	DEFAULT_CONSTRUCTORS_AND_MOVE(ComponentCharacterSheet_AttackCooldown,
								  {MV(baseCooldown) MV(lastTimestamp)});
};

struct ComponentCharacterSheet_Protection {
	int32_t armorPoints = 0;

	BITSCPP_BYTESTREAM_OP_DECLARATIONS();
	DEFAULT_CONSTRUCTORS_AND_MOVE(ComponentCharacterSheet_Protection,
								  {MV(armorPoints)});
};
