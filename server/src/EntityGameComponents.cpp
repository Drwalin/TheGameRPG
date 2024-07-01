#include "../../common/include/RegistryComponent.hpp"

#include "../include/EntityGameComponents.hpp"

GAME_REGISTER_ECS_COMPONENT_STATIC(ComponentOnUse, "TCBONU");
GAME_REGISTER_ECS_COMPONENT_STATIC(ComponentSingleDoorTransformStates, "SDST");

int RegisterEntityGameComponents() { return 0; }
