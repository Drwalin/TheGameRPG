#pragma once

#include <godot_cpp/variant/transform3d.hpp>

#include "ComponentBase.hpp"

namespace editor
{
class ComponentOpenableSingleDoor : public ComponentBase
{
	GDCLASS(ComponentOpenableSingleDoor, ComponentBase)
public: // Godot bound functions
	ComponentOpenableSingleDoor();
	virtual ~ComponentOpenableSingleDoor();
	static void _bind_methods();

	void _process(double dt) override;

	virtual void Serialize(icon7::ByteWriter &writer) override;

public: // variables
	bool openedState = false;
	bool get_openedState();
	void set_openedState(bool v);

	Transform3D transformClosed;
	Transform3D get_transformClosed();
	void set_transformClosed(Transform3D v);

	Transform3D relativeTransformOpened;
	Transform3D get_relativeTransformOpened();
	void set_relativeTransformOpened(Transform3D v);
};
} // namespace editor

