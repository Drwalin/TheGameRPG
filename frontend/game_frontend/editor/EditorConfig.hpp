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
#include <godot_cpp/classes/packed_scene.hpp>
#include <godot_cpp/classes/mesh.hpp>
#include <godot_cpp/classes/csg_primitive3d.hpp>
#include <godot_cpp/variant/utility_functions.hpp>
#include <godot_cpp/godot.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <gdextension_interface.h>
#include <godot_cpp/core/defs.hpp>
#include <godot_cpp/godot.hpp>
#include <godot_cpp/core/engine_ptrcall.hpp>
#include <godot_cpp/classes/mesh_instance3d.hpp>

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

#define REGISTER_PROPERTY_RESOURCE(CLASS, NAME, TYPE, RESOURCE_TYPE, SET_NAME) \
	{                                                                          \
		METHOD_NO_ARGS(CLASS, get_##NAME);                                     \
		METHOD_ARGS(CLASS, set_##NAME, SET_NAME);                              \
		ClassDB::add_property(                                                 \
			#CLASS,                                                            \
			PropertyInfo(TYPE, #NAME, PROPERTY_HINT_RESOURCE_TYPE,             \
						 RESOURCE_TYPE),                                       \
			TO_C_STRING_NAME(set_##NAME), TO_C_STRING_NAME(get_##NAME));       \
	}

using namespace godot;

namespace icon7
{
class ByteWriter;
}

namespace editor
{
class PrefabServerStaticMesh_Base;
class EntityBase;

inline const std::string RES_PREFIX = "res://assets/";

class GameEditorConfig : public Node3D
{
	GDCLASS(GameEditorConfig, Node3D)
public: // Godot bound functions
	GameEditorConfig() {}
	~GameEditorConfig() {}
	static void _bind_methods();

	virtual void _ready() override;
	virtual void _process(double dt) override;

	void SaveScene();
	// returns true if has any PrefabServer nodes
	void SerializeNode(Node *node, icon7::ByteWriter &writer);
	static Node3D *InstantiateGraphicMesh(Ref<Resource> res);

	void ForEachNode(Node *node, bool isInsideEntity,
					 void(func)(Node *, bool isInsideEntity));

public: // variables
	static bool render_graphic;
	bool get_render_graphic() { return render_graphic; }
	void set_render_graphic(bool v);

	static bool render_collision;
	bool get_render_collision() { return render_collision; }
	void set_render_collision(bool v);

	static bool render_triggers;
	bool get_render_triggers() { return render_triggers; }
	void set_render_triggers(bool v);
	
	bool save_file = false;
	bool get_save_file() { return save_file; }
	void set_save_file(bool v) { save_file = v; }

	String save_map_file_path;
	String get_save_map_file_path() { return save_map_file_path; }
	void set_save_map_file_path(String v) { save_map_file_path = v; }

public:
	std::vector<PrefabServerStaticMesh_Base *> staticToGoSecond;
	std::vector<EntityBase *> otherObjects;

private:
	int frameCounter = 0;
};

} // namespace editor
