#pragma once

#include <godot_cpp/classes/csg_box3d.hpp>

#include "ComponentBase.hpp"

namespace editor
{
class ComponentTrigger : public ComponentBase
{
	GDCLASS(ComponentTrigger, ComponentBase)
public: // Godot bound functions
	ComponentTrigger();
	virtual ~ComponentTrigger();
	static void _bind_methods();

	virtual void Serialize(icon7::ByteWriter &writer) override;

public: // variables
	String onTriggerEnter = "";
	String get_onTriggerEnter();
	void set_onTriggerEnter(String v);

	String onTriggerExit = "";
	String get_onTriggerExit();
	void set_onTriggerExit(String v);
};
} // namespace editor
