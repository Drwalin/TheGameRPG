#include "../../common/include/RegistryComponent.hpp"

#include "../include/EntityGameComponents.hpp"

GAME_REGISTER_ECS_COMPONENT_STATIC(ComponentTagCanBeUsed, "TCBU");
GAME_REGISTER_ECS_COMPONENT_STATIC(ComponentOnUse, "TCBONU");

GAME_REGISTER_ECS_COMPONENT_STATIC(ComponentOpenableState, "OPNST");
GAME_REGISTER_ECS_COMPONENT_STATIC(ComponentSingleDoorTransformStates, "SDST");