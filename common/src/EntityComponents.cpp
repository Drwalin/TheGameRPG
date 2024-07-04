
#include "../include/RegistryComponent.hpp"
#include "../include/Realm.hpp"

#include "../include/EntityComponents.hpp"

GAME_REGISTER_ECS_COMPONENT_STATIC(ComponentShape, "S");
GAME_REGISTER_ECS_COMPONENT_STATIC(ComponentName, "N");
GAME_REGISTER_ECS_COMPONENT_STATIC(ComponentMovementParameters, "MV");
GAME_REGISTER_ECS_COMPONENT_STATIC(ComponentModelName, "MN");
GAME_REGISTER_ECS_COMPONENT_STATIC(ComponentStaticTransform, "ST");
GAME_REGISTER_ECS_COMPONENT_STATIC(ComponentStaticCollisionShapeName, "SCS");

GAME_REGISTER_ECS_COMPONENT_STATIC_WITH_DESERIALIZE_CALLBACK(
	ComponentMovementState, "CP",
	[](flecs::entity entity, ComponentMovementState *state) {
		auto realm = *entity.world().get<RealmPtr>();
		state->timestamp = realm.realm->timer.currentTick;
	});

GAME_REGISTER_ECS_COMPONENT_STATIC_WITH_DESERIALIZE_CALLBACK(
	ComponentLastAuthoritativeMovementState, "AP",
	[](flecs::entity entity, auto *state) {
		auto realm = *entity.world().get<RealmPtr>();
		state->oldState.timestamp = realm.realm->timer.currentTick;
	});

int RegisterEntityEventQueueComponent(flecs::world &ecs);
int RegisterEntityComponentsCollisionWorld(flecs::world &ecs);

int RegisterEntityComponents(flecs::world &ecs) {
	LOG_INFO("Registering components");
	RegisterEntityEventQueueComponent(ecs);
	RegisterEntityComponentsCollisionWorld(ecs);
	ecs.component<ComponentShape>();
	ecs.component<ComponentName>();
	ecs.component<ComponentMovementParameters>();
	ecs.component<ComponentModelName>();
	ecs.component<ComponentStaticTransform>();
	ecs.component<ComponentStaticCollisionShapeName>();
	ecs.component<ComponentMovementState>();
	ecs.component<ComponentLastAuthoritativeMovementState>();
	ecs.component<ComponentCharacterSheet>();
	LOG_INFO("Done");
	return 0;
}
