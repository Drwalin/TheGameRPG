#pragma once

#include <godot_cpp/classes/csg_box3d.hpp>

#include "ComponentBase.hpp"

namespace editor
{
class ComponentOnUse : public ComponentBase
{
	GDCLASS(ComponentOnUse, ComponentBase)
public: // Godot bound functions
	ComponentOnUse();
	virtual ~ComponentOnUse();
	static void _bind_methods();

	virtual void Serialize(icon7::ByteWriter &writer) override;

public:
	String OnUse = "OpenableSingleDoor";
	String get_OnUse();
	void set_OnUse(String v);
};
} // namespace editor


