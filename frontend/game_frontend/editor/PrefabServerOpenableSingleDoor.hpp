#pragma once

#include <godot_cpp/variant/transform3d.hpp>

#include "PrefabServerStaticMesh.hpp"

namespace editor
{
class PrefabServerOpenableSingleDoor : public PrefabServerStaticMesh
{
	GDCLASS(PrefabServerOpenableSingleDoor, PrefabServerStaticMesh)
public: // Godot bound functions
	PrefabServerOpenableSingleDoor();
	virtual ~PrefabServerOpenableSingleDoor();
	static void _bind_methods();

	void _ready() override;
	void _process(double dt) override;

	virtual void Serialize(uint16_t higherLevelComponentsCount,
						   icon7::ByteWriter &writer) override;

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

	String OnUse = "OpenableSingleDoor";
	String get_OnUse();
	void set_OnUse(String v);
};
} // namespace editor
