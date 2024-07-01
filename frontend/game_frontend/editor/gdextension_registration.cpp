#include <godot_cpp/godot.hpp>
#include <godot_cpp/core/class_db.hpp>

#include <gdextension_interface.h>
#include <godot_cpp/core/defs.hpp>
#include <godot_cpp/godot.hpp>
#include <godot_cpp/core/engine_ptrcall.hpp>

#include <godot_cpp/classes/node2d.hpp>
#include <godot_cpp/variant/packed_byte_array.hpp>
#include <godot_cpp/classes/ref_counted.hpp>

#include "PrefabServerBase.hpp"
#include "PrefabServerStaticMesh.hpp"
#include "PrefabServerOpenableSingleDoor.hpp"
#include "EditorConfig.hpp"

static void register_gameplay_types(godot::ModuleInitializationLevel p_level)
{
	if (p_level !=
		godot::ModuleInitializationLevel::MODULE_INITIALIZATION_LEVEL_SCENE) {
		return;
	}
	godot::ClassDB::register_class<editor::PrefabServerBase>();
	godot::ClassDB::register_class<editor::PrefabServerStaticMesh>();
	godot::ClassDB::register_class<editor::PrefabServerOpenableSingleDoor>();
	godot::ClassDB::register_class<editor::GameEditorConfig>();
	// REGISTER CLASSES HERE LATER
}

static void unregister_gameplay_types(godot::ModuleInitializationLevel p_level)
{
	// DO NOTHING
}

extern "C" {

GDExtensionBool GDE_EXPORT
map_editor_library_init(GDExtensionInterfaceGetProcAddress p_interface,
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
