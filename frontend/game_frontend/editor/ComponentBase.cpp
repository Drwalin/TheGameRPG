#include <godot_cpp/variant/transform3d.hpp>
#include <godot_cpp/variant/basis.hpp>
#include <godot_cpp/classes/csg_primitive3d.hpp>
#include <godot_cpp/classes/csg_sphere3d.hpp>
#include <godot_cpp/classes/csg_box3d.hpp>
#include <godot_cpp/classes/csg_cylinder3d.hpp>

#include "ComponentBase.hpp"

namespace editor
{
ComponentBase::ComponentBase() {}
ComponentBase::~ComponentBase() {}
void ComponentBase::_bind_methods() {}

void ComponentBase::_ready() {}

void ComponentBase::_process(double dt) {}
} // namespace editor
