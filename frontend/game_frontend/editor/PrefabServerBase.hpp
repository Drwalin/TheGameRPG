#pragma once

#include <godot_cpp/classes/node2d.hpp>
#include <godot_cpp/classes/window.hpp>
#include <godot_cpp/classes/packed_scene.hpp>
#include <godot_cpp/classes/label3d.hpp>
#include <godot_cpp/classes/mesh.hpp>

#include <godot_cpp/variant/utility_functions.hpp>
#include <godot_cpp/godot.hpp>
#include <godot_cpp/core/class_db.hpp>

#include <gdextension_interface.h>
#include <godot_cpp/core/defs.hpp>
#include <godot_cpp/godot.hpp>
#include <godot_cpp/core/engine_ptrcall.hpp>
#include <godot_cpp/classes/scene_tree.hpp>
#include <godot_cpp/classes/mesh_instance3d.hpp>
#include <godot_cpp/classes/animation_tree.hpp>

#include <godot_cpp/classes/node3d.hpp>
#include <godot_cpp/variant/packed_byte_array.hpp>
#include <godot_cpp/classes/rigid_body3d.hpp>
#include <godot_cpp/classes/collision_shape3d.hpp>
#include <godot_cpp/classes/capsule_shape3d.hpp>

#include <glm/glm.hpp>

#include <icon7/ByteWriter.hpp>

#include "EditorConfig.hpp"

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

namespace editor
{

inline const std::string RES_PREFIX = "res://assets/";

class PrefabServerBase : public Node3D
{
	GDCLASS(PrefabServerBase, Node3D)
public: // Godot bound functions
	PrefabServerBase();
	virtual ~PrefabServerBase();
	static void _bind_methods() {}

	void ClearChildren();

	void _ready() override;
	void _process(double dt) override;

	virtual void Serialize(uint16_t higherLevelComponentsCount,
						   icon7::ByteWriter &writer);

	static String GetRandomString();

	template <typename TNode, typename TRes>
	void RecreateResourceRenderer(TNode **nodeStorage,
								  Ref<TRes> *resourceStorage, bool enable)
	{
		if (*nodeStorage) {
			remove_child((*nodeStorage));
			(*nodeStorage)->queue_free();
			(*nodeStorage) = nullptr;
		}
		if (GameEditorConfig::render_graphic) {
			if (resourceStorage->is_valid() && !resourceStorage->is_null()) {

				if constexpr (std::is_same_v<TNode, Node>) {
					Ref<PackedScene> packedScene = *resourceStorage;
					if (packedScene.is_null() == false &&
						packedScene.is_valid()) {
						(*nodeStorage) = packedScene->instantiate();
						(*nodeStorage)->set_name(GetRandomString());
						add_child((*nodeStorage));
						(*nodeStorage)->set_owner(this);
					}
				}

				Ref<Mesh> mesh = *resourceStorage;
				if (mesh.is_null() == false && mesh.is_valid()) {
					MeshInstance3D *m = new MeshInstance3D();
					m->set_mesh(mesh);
					(*nodeStorage) = m;
					(*nodeStorage)->set_name(GetRandomString());
					add_child((*nodeStorage));
					(*nodeStorage)->set_owner(this);
				}
			}
		}
		if (*nodeStorage) {
			GenerateTriCollisionForAll(*nodeStorage);
		}
	}

	void GenerateTriCollisionForAll(Node *node)
	{
		if (MeshInstance3D *mesh = Object::cast_to<MeshInstance3D>(node)) {
			mesh->create_trimesh_collision();
		}
		TypedArray<Node> children = node->get_children(true);
		for (int i = 0; i < children.size(); ++i) {
			GenerateTriCollisionForAll((Node *)(Object *)(children[i]));
		}
	}
};
} // namespace editor
