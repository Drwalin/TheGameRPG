#include "../../../common/include/RegistryComponent.hpp"
#include "../../../server/include/EntityGameComponents.hpp"

#include "../GameClientFrontend.hpp"

#include "EntityBase.hpp"
#include "ComponentOpenableSingleDoor.hpp"

namespace editor
{
ComponentOpenableSingleDoor::ComponentOpenableSingleDoor() {}
ComponentOpenableSingleDoor::~ComponentOpenableSingleDoor() {}

void ComponentOpenableSingleDoor::Serialize(icon7::ByteWriter &writer)
{
	Transform3D a = transformClosed;
	Transform3D b = transformClosed * relativeTransformOpened;

	Transform3D tmp =
		Object::cast_to<EntityBase>(get_parent())->get_transform();
	Object::cast_to<EntityBase>(get_parent())->set_transform(a);
	a = Object::cast_to<EntityBase>(get_parent())->get_global_transform();
	Object::cast_to<EntityBase>(get_parent())->set_transform(b);
	b = Object::cast_to<EntityBase>(get_parent())->get_global_transform();
	Object::cast_to<EntityBase>(get_parent())->set_transform(tmp);

	ComponentSingleDoorTransformStates transforms;
	transforms.transformClosed = ToGame(a);
	transforms.transformOpen = ToGame(b);
	reg::Registry::SerializePersistent(GameClientFrontend::singleton->realm,
									   transforms, writer);
}

void ComponentOpenableSingleDoor::_process(double dt)
{
	if (openedState == false) {
		transformClosed =
			Object::cast_to<EntityBase>(get_parent())->get_transform();
	} else {
		relativeTransformOpened =
			transformClosed.affine_inverse() *
			Object::cast_to<EntityBase>(get_parent())->get_transform();
	}
}

void ComponentOpenableSingleDoor::set_openedState(bool v)
{
	if (openedState == v) {
		return;
	}
	openedState = v;

	if (openedState) {
		Object::cast_to<EntityBase>(get_parent())
			->set_transform(transformClosed * relativeTransformOpened);
	} else {
		Object::cast_to<EntityBase>(get_parent())
			->set_transform(transformClosed);
	}
}

void ComponentOpenableSingleDoor::_bind_methods()
{
	REGISTER_PROPERTY(ComponentOpenableSingleDoor, openedState,
					  Variant::Type::BOOL, "IsOpened");
	REGISTER_PROPERTY(ComponentOpenableSingleDoor, transformClosed,
					  Variant::Type::TRANSFORM3D, "SecondaryTransform");
	REGISTER_PROPERTY(ComponentOpenableSingleDoor, relativeTransformOpened,
					  Variant::Type::TRANSFORM3D, "SecondaryTransform");
}

bool ComponentOpenableSingleDoor::get_openedState() { return openedState; }

Transform3D ComponentOpenableSingleDoor::get_transformClosed()
{
	return transformClosed;
}
void ComponentOpenableSingleDoor::set_transformClosed(Transform3D v)
{
	transformClosed = v;
}

Transform3D ComponentOpenableSingleDoor::get_relativeTransformOpened()
{
	return relativeTransformOpened;
}
void ComponentOpenableSingleDoor::set_relativeTransformOpened(Transform3D v)
{
	relativeTransformOpened = v;
}
} // namespace editor
