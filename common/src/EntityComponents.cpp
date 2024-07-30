
#include "../include/RegistryComponent.inl.hpp"
#include "../include/Realm.hpp"

#include "../include/EntityComponents.hpp"

GAME_REGISTER_ECS_COMPONENT_STATIC(ComponentShape, "S");
GAME_REGISTER_ECS_COMPONENT_STATIC(ComponentName, "N");
GAME_REGISTER_ECS_COMPONENT_STATIC(ComponentMovementParameters, "MV");
GAME_REGISTER_ECS_COMPONENT_STATIC(ComponentModelName, "MN");
GAME_REGISTER_ECS_COMPONENT_STATIC(ComponentStaticTransform, "ST");
GAME_REGISTER_ECS_COMPONENT_STATIC(ComponentStaticCollisionShapeName, "SCS");
GAME_REGISTER_ECS_COMPONENT_STATIC(ComponentMovementState, "CP");
GAME_REGISTER_ECS_COMPONENT_STATIC(ComponentLastAuthoritativeMovementState,
								   "AP");

int RegisterEntityEventQueueComponent(flecs::world &ecs);
int RegisterEntityComponentsCollisionWorld(flecs::world &ecs);
int RegisterEntityComponentsCharacterSheet(flecs::world &ecs);
int RegisterEntityComponentsInventory(flecs::world &ecs);

int RegisterEntityComponents(flecs::world &ecs)
{
	RegisterEntityEventQueueComponent(ecs);
	RegisterEntityComponentsCollisionWorld(ecs);
	RegisterEntityComponentsInventory(ecs);
	RegisterEntityComponentsCharacterSheet(ecs);
	ecs.component<ComponentShape>();
	ecs.component<ComponentName>();
	ecs.component<ComponentMovementParameters>();
	ecs.component<ComponentModelName>();
	ecs.component<ComponentStaticTransform>();
	ecs.component<ComponentStaticCollisionShapeName>();
	ecs.component<ComponentMovementState>();
	ecs.component<ComponentLastAuthoritativeMovementState>();

	reg::ComponentConstructor<ComponentMovementState>::singleton
		->callbackDeserializePersistent = [](class Realm *realm,
											 flecs::entity entity,
											 ComponentMovementState *state) {
		state->timestamp += realm->timer.currentTick;
	};
	reg::ComponentConstructor<ComponentMovementState>::singleton
		->callbackSerializePersistent = [](class Realm *realm,
										   ComponentMovementState *state) {
		state->timestamp -= realm->timer.currentTick;
	};

	reg::ComponentConstructor<ComponentLastAuthoritativeMovementState>::
		singleton->callbackDeserializePersistent =
		[](class Realm *realm, flecs::entity entity, auto *state) {
			state->oldState.timestamp += realm->timer.currentTick;
		};
	reg::ComponentConstructor<
		ComponentLastAuthoritativeMovementState>::singleton
		->callbackSerializePersistent = [](class Realm *realm, auto *state) {
		state->oldState.timestamp -= realm->timer.currentTick;
	};

	return 0;
}

BITSCPP_BYTESTREAM_OP_SYMMETRIC_DEFINITIONS(ComponentShape, {
	s.op(height);
	s.op(width);
});

BITSCPP_BYTESTREAM_OP_SYMMETRIC_DEFINITIONS(ComponentMovementState, {
	s.op(timestamp);
	s.op(pos);
	s.op(vel);
	s.op(rot);
	s.op(onGround);
});

BITSCPP_BYTESTREAM_OP_SYMMETRIC_DEFINITIONS(
	ComponentLastAuthoritativeMovementState, { s.op(oldState); });

BITSCPP_BYTESTREAM_OP_SYMMETRIC_DEFINITIONS(ComponentName, { s.op(name); });

BITSCPP_BYTESTREAM_OP_SYMMETRIC_DEFINITIONS(ComponentMovementParameters, {
	s.op(maxMovementSpeedHorizontal);
	s.op(stepHeight);
});

BITSCPP_BYTESTREAM_OP_SYMMETRIC_DEFINITIONS(ComponentModelName,
											{ s.op(modelName); });

BITSCPP_BYTESTREAM_OP_SYMMETRIC_DEFINITIONS(ComponentStaticTransform, {
	s.op(pos);
	s.op(rot);
	s.op(scale);
});

BITSCPP_BYTESTREAM_OP_SYMMETRIC_DEFINITIONS(ComponentStaticCollisionShapeName,
											{ s.op(shapeName); });
