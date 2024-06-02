#include "godot_cpp/classes/engine.hpp"
#include "godot_cpp/classes/engine_debugger.hpp"
#include "godot_cpp/classes/resource_loader.hpp"
#include <godot_cpp/classes/node2d.hpp>
#include <godot_cpp/classes/window.hpp>
#include <godot_cpp/classes/packed_scene.hpp>
#include <godot_cpp/classes/label3d.hpp>
#include <godot_cpp/classes/mesh.hpp>

#include "GodotGlm.hpp"

#include <icon7/Debug.hpp>

#include "EntityStaticPrefabScript.hpp"

#define METHOD_NO_ARGS(CLASS, NAME)                                            \
	ClassDB::bind_method(D_METHOD(#NAME), &CLASS::NAME);

#define METHOD_ARGS(CLASS, NAME, ...)                                          \
	ClassDB::bind_method(D_METHOD(#NAME, __VA_ARGS__), &CLASS::NAME);

EntityStaticPrefabScript::EntityStaticPrefabScript() {}

void EntityStaticPrefabScript::_bind_methods() {}

void EntityStaticPrefabScript::_ready()
{
	if (Engine::get_singleton()->is_editor_hint()) {
		return;
	}
	meshInstance = (MeshInstance3D *)(get_node<Node>("MeshInstance3D"));
}

void EntityStaticPrefabScript::_process(double dt)
{
	if (Engine::get_singleton()->is_editor_hint()) {
		return;
	}
}

void EntityStaticPrefabScript::Init(uint64_t localEntityId,
									const EntityModelName &model,
									EntityStaticTransform transform)
{
	if (localEntityId) {
		this->localEntityId = localEntityId;
	}
	meshInstance = (MeshInstance3D *)(get_node<Node>("MeshInstance3D"));

	ResourceLoader *rl = ResourceLoader::get_singleton();
	Ref<Mesh> mesh = rl->load(
		(std::string("res://assets/") + model.modelName).c_str(), "Mesh");

	meshInstance->set_mesh(mesh);
	set_position(ToGodot(transform.pos));
	set_rotation(ToGodot(transform.pos));
}

EntityStaticPrefabScript *EntityStaticPrefabScript::CreateNew()
{
	static Ref<PackedScene> prefab;
	if (prefab == nullptr) {
		auto x = ResourceLoader::get_singleton()->load(
			"res://prefabs/StaticEntity.tscn", "PackedScene");
		prefab = x;
	}
	auto ret = (EntityStaticPrefabScript *)(prefab->instantiate());
	return ret;
}
