#pragma once

#include <godot_cpp/variant/utility_functions.hpp>
#include <godot_cpp/godot.hpp>
#include <godot_cpp/core/class_db.hpp>

#include <gdextension_interface.h>
#include <godot_cpp/core/defs.hpp>
#include <godot_cpp/godot.hpp>
#include <godot_cpp/core/engine_ptrcall.hpp>
#include <godot_cpp/classes/scene_tree.hpp>
#include <godot_cpp/classes/node3d.hpp>
#include <godot_cpp/classes/mesh_instance3d.hpp>
#include <godot_cpp/classes/animation_tree.hpp>

#include <godot_cpp/classes/node2d.hpp>

#define METHOD_NO_ARGS(CLASS, NAME)                                            \
	ClassDB::bind_method(D_METHOD(#NAME), &CLASS::NAME);

#define METHOD_ARGS(CLASS, NAME, ...)                                          \
	ClassDB::bind_method(D_METHOD(#NAME, __VA_ARGS__), &CLASS::NAME);

#define TO_C_STRING_NAME(STR) #STR

#define REGISTER_PROPERTY(CLASS, NAME, TYPE, SET_NAME)                         \
	{                                                                          \
		METHOD_NO_ARGS(CLASS, get_##NAME);                                     \
		METHOD_ARGS(CLASS, set_##NAME, SET_NAME);                              \
		ClassDB::add_property(#CLASS, PropertyInfo(TYPE, #NAME),               \
							  TO_C_STRING_NAME(set_##NAME),                    \
							  TO_C_STRING_NAME(get_##NAME));                   \
	}

#define REGISTER_PROPERTY_WITH_HINT(CLASS, NAME, TYPE, HINT, HINT_STRING,      \
									SET_NAME)                                  \
	{                                                                          \
		METHOD_NO_ARGS(CLASS, get_##NAME);                                     \
		METHOD_ARGS(CLASS, set_##NAME, SET_NAME);                              \
		ClassDB::add_property(                                                 \
			#CLASS, PropertyInfo(TYPE, #NAME, HINT, HINT_STRING),              \
			TO_C_STRING_NAME(set_##NAME), TO_C_STRING_NAME(get_##NAME));       \
	}

using namespace godot;

namespace editor
{
class GameEditorConfig : public Node3D
{
	GDCLASS(GameEditorConfig, Node3D)
public: // Godot bound functions
	GameEditorConfig() {}
	~GameEditorConfig() {}
	static void _bind_methods()
	{
		REGISTER_PROPERTY(GameEditorConfig, render_graphic, Variant::Type::BOOL,
						  "render");
		REGISTER_PROPERTY(GameEditorConfig, render_collision,
						  Variant::Type::BOOL, "render");
		REGISTER_PROPERTY_WITH_HINT(
			GameEditorConfig, save_map_file_path, Variant::Type::STRING,
			PropertyHint::PROPERTY_HINT_GLOBAL_SAVE_FILE, "",
			"save_map_file_path");
		REGISTER_PROPERTY(GameEditorConfig, save_scene, Variant::Type::BOOL,
						  "save_scene");
	}

	virtual void _process(double dt) override;

	void SaveScene();

public: // variables
	static bool render_graphic;
	bool get_render_graphic() { return render_graphic; }
	void set_render_graphic(bool v) { render_graphic = v; }

	static bool render_collision;
	bool get_render_collision() { return render_collision; }
	void set_render_collision(bool v) { render_collision = v; }

	bool save_scene = false;
	bool get_save_scene() { return save_scene; }
	void set_save_scene(bool v) { save_scene = v; }

	String save_map_file_path;
	String get_save_map_file_path() { return save_map_file_path; }
	void set_save_map_file_path(String v) { save_map_file_path = v; }
};

} // namespace editor
