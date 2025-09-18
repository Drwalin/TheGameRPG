#include "../../ICon7/include/icon7/ByteBuffer.hpp"

#include "../include/RegistryComponent.inl.hpp"
#include "../include/Realm.hpp"

#include "../include/EntityComponents.hpp"

#include "../include/CollisionShapeSerialization.hpp"

int RegisterEntityEventQueueComponent(flecs::world &ecs);
int RegisterEntityComponentsCollisionWorld(flecs::world &ecs);
int RegisterEntityComponentsCharacterSheet(flecs::world &ecs);
int RegisterEntityComponentsInventory(flecs::world &ecs);

int RegisterEntityComponents(flecs::world &ecs)
{
	ecs.component<_InternalComponent_ComponentConstructorBasePointer>();
	REGISTER_COMPONENT_NO_SERIALISATION(ecs, TagAllEntity);

	RegisterEntityEventQueueComponent(ecs);
	RegisterEntityComponentsCollisionWorld(ecs);
	RegisterEntityComponentsInventory(ecs);
	RegisterEntityComponentsCharacterSheet(ecs);

	REGISTER_COMPONENT_FOR_ECS_WORLD(ecs, ComponentShape, "S");
	REGISTER_COMPONENT_FOR_ECS_WORLD(ecs, ComponentName, "N");
	REGISTER_COMPONENT_FOR_ECS_WORLD(ecs, ComponentMovementParameters, "MV");
	REGISTER_COMPONENT_FOR_ECS_WORLD(ecs, ComponentModelName, "MN");
	REGISTER_COMPONENT_FOR_ECS_WORLD(ecs, ComponentStaticTransform, "ST");
	REGISTER_COMPONENT_FOR_ECS_WORLD(ecs, ComponentCollisionShape, "CS");
	REGISTER_COMPONENT_FOR_ECS_WORLD(ecs, ComponentMovementState, "CP");
	REGISTER_COMPONENT_FOR_ECS_WORLD(
		ecs, ComponentLastAuthoritativeMovementState, "AP");

	REGISTER_COMPONENT_NO_SERIALISATION(ecs, ComponentEventsQueue);

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
	s.op(trans);
	s.op(scale);
});

BITSCPP_BYTESTREAM_OP_SYMMETRIC_DEFINITIONS(ComponentCollisionShape,
											{ s.op(shape); s.op(mask); });

