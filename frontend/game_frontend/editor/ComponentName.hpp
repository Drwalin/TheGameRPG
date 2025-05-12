#pragma once

#include "ComponentBase.hpp"

namespace editor
{
class ComponentName : public ComponentBase
{
	GDCLASS(ComponentName, ComponentBase)
public:
	ComponentName();
	virtual ~ComponentName();
	static void _bind_methods();

	virtual void Serialize(icon7::ByteWriter &writer) override;

public:
	String name = "";
	String get_name() { return name; }
	void set_name(String v) { name = v; }
};
} // namespace editor
