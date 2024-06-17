
#include "../include/EntityComponents.hpp"
#include "../include/RegistryComponent.hpp"

GAME_REGISTER_ECS_COMPONENT_STATIC(ComponentShape, "S");
GAME_REGISTER_ECS_COMPONENT_STATIC(ComponentMovementState, "CP");
GAME_REGISTER_ECS_COMPONENT_STATIC(ComponentLastAuthoritativeMovementState, "AP");
GAME_REGISTER_ECS_COMPONENT_STATIC(ComponentName, "N");
GAME_REGISTER_ECS_COMPONENT_STATIC(ComponentMovementParameters, "MV");
GAME_REGISTER_ECS_COMPONENT_STATIC(ComponentModelName, "MN");
GAME_REGISTER_ECS_COMPONENT_STATIC(ComponentStaticTransform, "ST");
GAME_REGISTER_ECS_COMPONENT_STATIC(ComponentStaticCollisionShapeName, "SCS");

void DDD() {}
