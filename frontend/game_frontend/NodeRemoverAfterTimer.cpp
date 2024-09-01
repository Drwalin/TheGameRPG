#include <godot_cpp/classes/engine.hpp>
#include <godot_cpp/classes/engine_debugger.hpp>
#include <godot_cpp/classes/resource_loader.hpp>

#include <icon7/Debug.hpp>

#include "NodeRemoverAfterTimer.hpp"

#define METHOD_NO_ARGS(CLASS, NAME)                                            \
	ClassDB::bind_method(D_METHOD(#NAME), &CLASS::NAME);

#define METHOD_ARGS(CLASS, NAME, ...)                                          \
	ClassDB::bind_method(D_METHOD(#NAME, __VA_ARGS__), &CLASS::NAME);

NodeRemoverAfterTimer::NodeRemoverAfterTimer() {}

NodeRemoverAfterTimer::~NodeRemoverAfterTimer() {}

void NodeRemoverAfterTimer::_bind_methods() {}

void NodeRemoverAfterTimer::_ready() {}

void NodeRemoverAfterTimer::_process(double dt)
{
	remainingSeconds -= dt;
	if (remainingSeconds <= 0) {
		this->get_parent()->queue_free();
	}
}
