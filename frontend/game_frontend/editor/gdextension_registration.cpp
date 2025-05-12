#include <godot_cpp/godot.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <gdextension_interface.h>
#include <godot_cpp/classes/display_server.hpp>
#include <godot_cpp/core/defs.hpp>
#include <godot_cpp/godot.hpp>
#include <godot_cpp/core/engine_ptrcall.hpp>

#include <godot_cpp/classes/node2d.hpp>
#include <godot_cpp/variant/packed_byte_array.hpp>
#include <godot_cpp/classes/ref_counted.hpp>
#include <godot_cpp/classes/engine.hpp>
#include <godot_cpp/classes/os.hpp>

#include <icon7/Debug.hpp>

#include "EntityBase.hpp"

#include "ComponentName.hpp"
#include "ComponentTrigger.hpp"
#include "ComponentTeleport.hpp"
#include "ComponentOnUse.hpp"
#include "ComponentArmorPoints.hpp"
#include "ComponentOpenableSingleDoor.hpp"
#include "ComponentBase.hpp"
#include "ComponentAITick.hpp"
#include "ComponentHealth.hpp"
#include "ComponentSpawner.hpp"
#include "ComponentHealthRegen.hpp"
#include "ComponentCharacterSheet.hpp"

#include "EditorConfig.hpp"

static void register_gameplay_types(godot::ModuleInitializationLevel p_level)
{
	if (p_level !=
		godot::ModuleInitializationLevel::MODULE_INITIALIZATION_LEVEL_SCENE) {
		return;
	}

	godot::ClassDB::register_abstract_class<editor::EntityBase>();
	godot::ClassDB::register_abstract_class<editor::ComponentBase>();
	godot::ClassDB::register_class<editor::ComponentAITick>();
	godot::ClassDB::register_class<editor::ComponentArmorPoints>();
	godot::ClassDB::register_class<editor::ComponentCharacterSheet>();
	godot::ClassDB::register_class<editor::ComponentHealth>();
	godot::ClassDB::register_class<editor::ComponentHealthRegen>();
	godot::ClassDB::register_class<editor::ComponentName>();
	godot::ClassDB::register_class<editor::ComponentOnUse>();
	godot::ClassDB::register_class<editor::ComponentOpenableSingleDoor>();
	godot::ClassDB::register_class<editor::ComponentSpawner>();
	godot::ClassDB::register_class<editor::ComponentTeleport>();
	godot::ClassDB::register_class<editor::ComponentTrigger>();
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
