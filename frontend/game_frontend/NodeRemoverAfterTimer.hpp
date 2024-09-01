#pragma once

#include <godot_cpp/variant/utility_functions.hpp>
#include <godot_cpp/godot.hpp>
#include <godot_cpp/core/class_db.hpp>

#include <gdextension_interface.h>
#include <godot_cpp/core/defs.hpp>
#include <godot_cpp/godot.hpp>
#include <godot_cpp/core/engine_ptrcall.hpp>
#include <godot_cpp/classes/node3d.hpp>

using namespace godot;

class NodeRemoverAfterTimer : public Node3D
{
	GDCLASS(NodeRemoverAfterTimer, Node3D)
public: // Godot bound functions
	NodeRemoverAfterTimer();
	~NodeRemoverAfterTimer();
	static void _bind_methods();

	void _ready() override;
	void _process(double dt) override;

public: // variables
	float remainingSeconds = 1.0;
};
