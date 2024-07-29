#include <godot_cpp/classes/engine.hpp>
#include <godot_cpp/classes/engine_debugger.hpp>
#include <godot_cpp/classes/resource_loader.hpp>
#include <godot_cpp/classes/node2d.hpp>
#include <godot_cpp/classes/window.hpp>
#include <godot_cpp/classes/packed_scene.hpp>
#include <godot_cpp/classes/label3d.hpp>
#include <godot_cpp/classes/mesh.hpp>

#include <icon7/Debug.hpp>

#include "GodotGlm.hpp"

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
}

void EntityStaticPrefabScript::_process(double dt)
{
	if (Engine::get_singleton()->is_editor_hint()) {
		return;
	}
}

void EntityStaticPrefabScript::Init(uint64_t localEntityId,
									const ComponentModelName &model,
									ComponentStaticTransform transform)
{
	SetTransform(transform);

	while (get_child_count(true) > 0) {
		Node *child = get_child(0, true);
		remove_child(child);
		child->queue_free();
	}

	if (localEntityId) {
		this->localEntityId = localEntityId;
	} else {
		LOG_ERROR(
			"trying to initialize EntityStaticPrefabScript with entityId == 0");
	}

	ResourceLoader *rl = ResourceLoader::get_singleton();
	Ref<Resource> resource =
		rl->load((std::string("res://assets/") + model.modelName).c_str());

	if (resource.is_null() == false && resource.is_valid()) {
		Ref<Mesh> mesh = resource;
		Ref<PackedScene> packedScene = resource;
		if (mesh.is_valid() && mesh.is_null() == false) {
			MeshInstance3D *meshInstance = new MeshInstance3D();
			meshInstance->set_mesh(mesh);
			add_child(meshInstance);
		}
		if (packedScene.is_valid() && packedScene.is_null() == false) {
			Node *node = packedScene->instantiate();
			add_child(node);
		}
	}
}

void EntityStaticPrefabScript::SetTransform(
	const ComponentStaticTransform &transform)
{
	set_transform(
		Transform3D{Basis(ToGodot(transform.rot)), ToGodot(transform.pos)});
	set_scale(ToGodot(transform.scale));
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
