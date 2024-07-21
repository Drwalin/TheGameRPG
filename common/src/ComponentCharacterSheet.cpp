#include "../include/RegistryComponent.inl.hpp"

#include "../include/ComponentCharacterSheet.hpp"

GAME_REGISTER_ECS_COMPONENT_STATIC(ComponentCharacterSheet, "CS");
GAME_REGISTER_ECS_COMPONENT_STATIC(ComponentDynamicCharacterSheet, "DCS");

int RegisterEntityComponentsCharacterSheet(flecs::world &ecs)
{
	ecs.component<ComponentCharacterSheet>();
	ecs.component<ComponentDynamicCharacterSheet>();
	return 0;
}

BITSCPP_BYTESTREAM_OP_SYMMETRIC_DEFINITIONS(ComponentCharacterSheet, {
	s.op(useRange);
	s.op(maxHP);
});

BITSCPP_BYTESTREAM_OP_SYMMETRIC_DEFINITIONS(ComponentDynamicCharacterSheet, {
	s.op(useRange);
	s.op(maxHP);
	s.op(hp);
});
