#include <godot_cpp/classes/engine.hpp>
#include <godot_cpp/classes/engine_debugger.hpp>
#include <godot_cpp/classes/resource_loader.hpp>

#include "NodeRemoverAfterTimer.hpp"

#define METHOD_NO_ARGS(CLASS, NAME)                                            \
	ClassDB::bind_method(D_METHOD(#NAME), &CLASS::NAME);

#define METHOD_ARGS(CLASS, NAME, ...)                                          \
	ClassDB::bind_method(D_METHOD(#NAME, __VA_ARGS__), &CLASS::NAME);

NodeRemoverAfterTimer::NodeRemoverAfterTimer()
{
	UtilityFunctions::print("Construct: %.2f", remainingSeconds);
}

NodeRemoverAfterTimer::~NodeRemoverAfterTimer()
{
	UtilityFunctions::print("Destruct: %.2f", remainingSeconds);
}

void NodeRemoverAfterTimer::_bind_methods() {}

void NodeRemoverAfterTimer::_process(double dt)
{
	UtilityFunctions::print("time left: %.2f", remainingSeconds);
	remainingSeconds -= dt;
	if (remainingSeconds <= 0) {
		UtilityFunctions::print("Destroying at: %.2f", remainingSeconds);
		this->get_parent()->queue_free();
	}
}
