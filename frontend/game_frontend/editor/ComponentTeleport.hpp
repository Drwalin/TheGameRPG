#pragma once

#include <godot_cpp/classes/csg_box3d.hpp>

#include "ComponentBase.hpp"

namespace editor
{
class ComponentTeleport : public ComponentBase
{
	GDCLASS(ComponentTeleport, ComponentBase)
public: // Godot bound functions
	ComponentTeleport();
	virtual ~ComponentTeleport();
	static void _bind_methods();

	virtual void Serialize(icon7::ByteWriter &writer) override;

public:
	String teleportRealm = "";
	String get_teleportRealm();
	void set_teleportRealm(String v);

	Vector3 teleportPosition = {0, 0, 0};
	Vector3 get_teleportPosition();
	void set_teleportPosition(Vector3 v);
};
} // namespace editor

