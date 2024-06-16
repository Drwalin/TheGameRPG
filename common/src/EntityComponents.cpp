
#include "../include/EntityComponents.hpp"
#include "../include/RegistryComponent.hpp"

using namespace reg;

SpecializedRegistry entityPublicRegistry;
SpecializedRegistry entityPrivateRegistry;

GAME_REGISTER_ECS_COMPONENT_STATIC(ComponentShape, &entityPublicRegistry,
								   &entityPrivateRegistry);
GAME_REGISTER_ECS_COMPONENT_STATIC(ComponentMovementState,
								   &entityPublicRegistry,
								   &entityPrivateRegistry);
GAME_REGISTER_ECS_COMPONENT_STATIC(ComponentLastAuthoritativeMovementState,
								   &entityPublicRegistry,
								   &entityPrivateRegistry);
GAME_REGISTER_ECS_COMPONENT_STATIC(ComponentName, &entityPublicRegistry,
								   &entityPrivateRegistry);
GAME_REGISTER_ECS_COMPONENT_STATIC(ComponentMovementParameters,
								   &entityPublicRegistry,
								   &entityPrivateRegistry);
GAME_REGISTER_ECS_COMPONENT_STATIC(ComponentModelName, &entityPublicRegistry,
								   &entityPrivateRegistry);
GAME_REGISTER_ECS_COMPONENT_STATIC(ComponentStaticTransform,
								   &entityPublicRegistry,
								   &entityPrivateRegistry);
GAME_REGISTER_ECS_COMPONENT_STATIC(ComponentStaticCollisionShapeName,
								   &entityPublicRegistry,
								   &entityPrivateRegistry);
