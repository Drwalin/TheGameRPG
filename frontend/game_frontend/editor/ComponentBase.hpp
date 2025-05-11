#pragma once

#include "../../../common/include/EntityComponents.hpp"

#include "EditorConfig.hpp"

using namespace godot;

namespace editor
{
class ComponentBase : public Node3D
{
	GDCLASS(ComponentBase, Node3D)
public:
	ComponentBase();
	virtual ~ComponentBase();
	static void _bind_methods();

	void _ready() override;
	void _process(double dt) override;

	virtual void Serialize(icon7::ByteWriter &writer) = 0;
};

} // namespace editor
