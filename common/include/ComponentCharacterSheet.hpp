#pragma once

#include "ComponentsUtility.hpp"

struct ComponentCharacterSheet {
	float useRange = 4.0f;
	int maxHP = 10;

	BITSCPP_BYTESTREAM_OP_DECLARATIONS();
	DEFAULT_CONSTRUCTORS_AND_MOVE(ComponentCharacterSheet, MV(useRange));
};

struct ComponentDynamicCharacterSheet {
	float useRange = 4.0f;
	int maxHP = 10;
	int hp = 10;

	BITSCPP_BYTESTREAM_OP_DECLARATIONS();
	DEFAULT_CONSTRUCTORS_AND_MOVE(ComponentDynamicCharacterSheet,
								  {MV(useRange) MV(maxHP) MV(hp)});
};
