#include <godot_cpp/godot.hpp>
#include <godot_cpp/core/class_db.hpp>

#include <gdextension_interface.h>
#include <godot_cpp/core/defs.hpp>
#include <godot_cpp/godot.hpp>
#include <godot_cpp/core/engine_ptrcall.hpp>

#include <godot_cpp/classes/node2d.hpp>
#include <godot_cpp/variant/packed_byte_array.hpp>
#include <godot_cpp/classes/ref_counted.hpp>
#include <godot_cpp/classes/engine.hpp>

#include "GameFrontend.hpp"
#include "EntityPrefabScript.hpp"
#include "EntityStaticPrefabScript.hpp"
#include "NodeRemoverAfterTimer.hpp"
#include "AsyncLoadedPlaceholder.hpp"

int RegisterFrontendEntityComponents(flecs::world &ecs);

static void register_gameplay_types(godot::ModuleInitializationLevel p_level)
{
	if (p_level !=
		godot::ModuleInitializationLevel::MODULE_INITIALIZATION_LEVEL_SCENE) {
		return;
	}

	{
		flecs::world ecs;
		RegisterFrontendEntityComponents(ecs);
	}

	godot::ClassDB::register_class<AsyncLoadedPlaceholder3D>();
	godot::ClassDB::register_class<GameFrontend>();
	godot::ClassDB::register_class<EntityPrefabScript>();
	godot::ClassDB::register_class<EntityStaticPrefabScript>();
	godot::ClassDB::register_class<NodeRemoverAfterTimer>();
	// REGISTER CLASSES HERE LATER
}

static void unregister_gameplay_types(godot::ModuleInitializationLevel p_level)
{
	// DO NOTHING
}

extern "C" {

GDExtensionBool GDE_EXPORT
game_frontend_library_init(GDExtensionInterfaceGetProcAddress p_interface,
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
