#include "../../../common/include/RegistryComponent.hpp"
#include "../../../server/include/EntityGameComponents.hpp"

#include "PrefabServerOpenableSingleDoor.hpp"

namespace editor
{
PrefabServerOpenableSingleDoor::PrefabServerOpenableSingleDoor() {}
PrefabServerOpenableSingleDoor::~PrefabServerOpenableSingleDoor() {}

void PrefabServerOpenableSingleDoor::Serialize(
	uint16_t higherLevelComponentsCount, icon7::ByteWriter &writer)
{
	PrefabServerStaticMesh::Serialize(higherLevelComponentsCount + 2, writer);

	std::string onUseCallbackName = OnUse.utf8().ptr();
	named_callbacks::registry_entries::OnUse onUseEntry{
		onUseCallbackName, onUseCallbackName, {}, nullptr, nullptr};
	ComponentOnUse onUse{&onUseEntry};
	reg::Registry::Serialize(onUse, writer);

	Transform3D a = transformClosed;
	Transform3D b = transformClosed * relativeTransformOpened;

	Transform3D tmp = get_transform();
	set_transform(a);
	a = get_global_transform();
	set_transform(b);
	b = get_global_transform();
	set_transform(tmp);

	ComponentSingleDoorTransformStates transforms{ToGame(a), ToGame(b)};
	reg::Registry::Serialize(transforms, writer);
}

void PrefabServerOpenableSingleDoor::_process(double dt)
{
	PrefabServerStaticMesh::_process(dt);

	if (openedState == false) {
		transformClosed = get_transform();
	} else {
		relativeTransformOpened = transformClosed.inverse() * get_transform();
	}
}

void PrefabServerOpenableSingleDoor::set_openedState(bool v)
{
	if (openedState == v) {
		return;
	}
	openedState = v;

	if (openedState) {
		set_transform(transformClosed * relativeTransformOpened);
	} else {
		set_transform(transformClosed);
	}
}

void PrefabServerOpenableSingleDoor::_bind_methods()
{
	REGISTER_PROPERTY(PrefabServerOpenableSingleDoor, openedState,
					  Variant::Type::BOOL, "IsOpened");
	REGISTER_PROPERTY(PrefabServerOpenableSingleDoor, transformClosed,
					  Variant::Type::TRANSFORM3D, "SecondaryTransform");
	REGISTER_PROPERTY(PrefabServerOpenableSingleDoor, relativeTransformOpened,
					  Variant::Type::TRANSFORM3D, "SecondaryTransform");
}

void PrefabServerOpenableSingleDoor::_ready()
{
	PrefabServerStaticMesh::_ready();
}

bool PrefabServerOpenableSingleDoor::get_openedState() { return openedState; }

Transform3D PrefabServerOpenableSingleDoor::get_transformClosed()
{
	return transformClosed;
}
void PrefabServerOpenableSingleDoor::set_transformClosed(Transform3D v)
{
	transformClosed = v;
}

Transform3D PrefabServerOpenableSingleDoor::get_relativeTransformOpened()
{
	return relativeTransformOpened;
}
void PrefabServerOpenableSingleDoor::set_relativeTransformOpened(Transform3D v)
{
	relativeTransformOpened = v;
}

String PrefabServerOpenableSingleDoor::get_OnUse() { return OnUse; }
void PrefabServerOpenableSingleDoor::set_OnUse(String v) { OnUse = v; }

} // namespace editor
