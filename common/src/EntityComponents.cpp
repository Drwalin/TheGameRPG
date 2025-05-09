
#include "../include/RegistryComponent.inl.hpp"
#include "../include/Realm.hpp"

#include "../include/EntityComponents.hpp"

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
	REGISTER_COMPONENT_NO_SERIALISATION(ecs, ComponentBulletCollisionObject);

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

BITSCPP_BYTESTREAM_OP_SYMMETRIC_DEFINITIONS(ComponentCollisionShape,
											{ s.op(shape); });

bitscpp::ByteReader<true> &
__InnerShape::__ByteStream_op(bitscpp::ByteReader<true> &s)
{
	shape = {};
	s.op(type);
	s.op(trans);
	switch (type) {
	case VERTBOX:
		{
			Collision3D::VertBox v;
			s.op(v);
			shape = v;
		}
		break;
	case CYLINDER:
		{
			Collision3D::Cylinder v;
			s.op(v);
			shape = v;
		}
		break;
	case HEIGHTMAP:
		{
			Collision3D::HeightMap<float> v;
			s.op(v);
			shape = v;
		}
		break;
	case COMPOUND_SHAPE:
		{
			CompoundShape v;
			s.op(v);
			shape = v;
		}
		break;
	default:
		LOG_FATAL("Invalid collision shape type: %u", (uint32_t)type);
	}
	return s;
}
bitscpp::ByteWriter<icon7::ByteBuffer> &
__InnerShape::__ByteStream_op(bitscpp::ByteWriter<icon7::ByteBuffer> &s)
{
	switch(shape.index()) {
		case 0:
			LOG_FATAL("CollisionShape is empty but should not be");
			s.op(NONE);
			break;
		case 1:
			s.op(VERTBOX);
			s.op(std::get<0>(shape));
			break;
		case 2:
			s.op(CYLINDER);
			s.op(std::get<1>(shape));
			break;
		case 3:
			s.op(HEIGHTMAP);
			s.op(std::get<2>(shape));
			break;
		case 4:
			s.op(COMPOUND_SHAPE);
			s.op(std::get<3>(shape));
			break;
	}
	return s;
}

bitscpp::ByteReader<true> &
CompoundShape::__ByteStream_op(bitscpp::ByteReader<true> &s)
{
	if (shapes.get() == nullptr) {
		shapes = std::make_shared<std::vector<__InnerShape>>();
	}
	s.op(*shapes.get());
	return s;
}
bitscpp::ByteWriter<icon7::ByteBuffer> &
CompoundShape::__ByteStream_op(bitscpp::ByteWriter<icon7::ByteBuffer> &s)
{
	if (shapes.get() == nullptr) {
		LOG_FATAL("Empty compound shape");
		s.op((uint32_t)0);
	} else {
		if (shapes->size() == 0) {
			LOG_FATAL("Empty compound shape");
		}
		s.op(*shapes.get());
	}
	return s;
}
