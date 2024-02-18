
#include "godot_cpp/variant/utility_functions.hpp"
#include <godot_cpp/godot.hpp>
#include <godot_cpp/core/class_db.hpp>

#include <gdextension_interface.h>
#include <godot_cpp/core/defs.hpp>
#include <godot_cpp/godot.hpp>
#include <godot_cpp/core/engine_ptrcall.hpp>

#include <godot_cpp/classes/node2d.hpp>
#include <godot_cpp/variant/packed_byte_array.hpp>
#include <godot_cpp/classes/ref_counted.hpp>

#define METHOD_NO_ARGS(CLASS, NAME)                                            \
	godot::ClassDB::bind_method(godot::D_METHOD(#NAME), &CLASS::NAME);

#define METHOD_ARGS(CLASS, NAME, ...)                                          \
	godot::ClassDB::bind_method(godot::D_METHOD(#NAME, __VA_ARGS__),           \
								&CLASS::NAME);

class TestClassGaming : public godot::Node
{
	GDCLASS(TestClassGaming, Node)
public:
	static void _bind_methods()
	{
		METHOD_ARGS(TestClassGaming, Func, "a", "b");
	}
	
	void _enter_tree() override
	{
		if (godot::Engine::get_singleton()->is_editor_hint())
			set_process_mode(Node::ProcessMode::PROCESS_MODE_DISABLED);
	}
	
	void _process(double_t dt) override
	{
		t += dt;
		if (t > 0.25) {
			godot::UtilityFunctions::print("Printing");
			t -= 0.25;
		}
	}
	
	int64_t Func(int64_t a, int64_t b)
	{
		return a+b;
	}
	
private:
	float t=0;
};




void register_gameplay_types(godot::ModuleInitializationLevel p_level)
{
	if (p_level !=
		godot::ModuleInitializationLevel::MODULE_INITIALIZATION_LEVEL_SCENE) {
		return;
	}

	godot::ClassDB::register_class<TestClassGaming>();
	// REGISTER CLASSES HERE LATER
}

void unregister_gameplay_types(godot::ModuleInitializationLevel p_level)
{
	// DO NOTHING
}

extern "C" {

GDExtensionBool GDE_EXPORT
game_client_library_init(GDExtensionInterfaceGetProcAddress p_interface,
				   GDExtensionClassLibraryPtr p_library,
				   GDExtensionInitialization *r_initialization)
{
	godot::GDExtensionBinding::InitObject init_object(p_interface, p_library,
													  r_initialization);

	init_object.register_initializer(register_gameplay_types);
	init_object.register_terminator(unregister_gameplay_types);
	init_object.set_minimum_library_initialization_level(
		godot::ModuleInitializationLevel::MODULE_INITIALIZATION_LEVEL_SCENE);

	return init_object.init();
}
}
