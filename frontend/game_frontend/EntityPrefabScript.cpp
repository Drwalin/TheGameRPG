#include <godot_cpp/classes/engine.hpp>
#include <godot_cpp/classes/engine_debugger.hpp>
#include <godot_cpp/classes/resource_loader.hpp>
#include <godot_cpp/classes/node2d.hpp>
#include <godot_cpp/classes/window.hpp>
#include <godot_cpp/classes/packed_scene.hpp>
#include <godot_cpp/classes/label3d.hpp>

#include <icon7/Debug.hpp>

#include "GameFrontend.hpp"

#include "GodotGlm.hpp"

#include "EntityPrefabScript.hpp"

#define METHOD_NO_ARGS(CLASS, NAME)                                            \
	ClassDB::bind_method(D_METHOD(#NAME), &CLASS::NAME);

#define METHOD_ARGS(CLASS, NAME, ...)                                          \
	ClassDB::bind_method(D_METHOD(#NAME, __VA_ARGS__), &CLASS::NAME);

EntityPrefabScript::EntityPrefabScript() {}

EntityPrefabScript::~EntityPrefabScript() {}

void EntityPrefabScript::_bind_methods()
{
	METHOD_NO_ARGS(EntityPrefabScript, GetGameFrontend);

	METHOD_NO_ARGS(EntityPrefabScript, IsPlayer);
	METHOD_NO_ARGS(EntityPrefabScript, GetLocalEntityId);
	METHOD_NO_ARGS(EntityPrefabScript, GetRotation);
	METHOD_NO_ARGS(EntityPrefabScript, GetPosition);
	METHOD_NO_ARGS(EntityPrefabScript, GetVelocity);
	METHOD_NO_ARGS(EntityPrefabScript, GetOnGround);
	METHOD_NO_ARGS(EntityPrefabScript, GetAnimationTree);

	METHOD_ARGS(EntityPrefabScript, _my_internal_process, "deltaTime");
}

void EntityPrefabScript::_ready()
{
	if (Engine::get_singleton()->is_editor_hint()) {
		return;
	}

	frontend = (GameFrontend *)(get_tree()->get_root()->get_node_or_null(
		"gameFrontend"));

	nodeContainingModel = (Node *)(get_node<Node>("Mesh"));
}

void EntityPrefabScript::_process(double dt) { _my_internal_process(dt); }

void EntityPrefabScript::_my_internal_process(double dt)
{
	if (Engine::get_singleton()->is_editor_hint()) {
		return;
	}

	flecs::entity entity = frontend->client->realm->Entity(localEntityId);
	if (entity.is_alive() == false) {
		LOG_ERROR("Entity %lu is not alive/not present in ecs", localEntityId);
		return;
	}
	auto state = entity.get<ComponentMovementState>();
	if (state) {
		SetRotation(state->rot);
		SetPosition(state->pos);
	} else {
		LOG_ERROR("Entity %lu has no movement state", localEntityId);
	}
}

void EntityPrefabScript::Init(uint64_t localEntityId)
{
	if (localEntityId) {
		this->localEntityId = localEntityId;
	}
}

void EntityPrefabScript::SetRotation(glm::vec3 rot)
{
	set_rotation(ToGodot({0, rot.y, 0}));
}

void EntityPrefabScript::SetPosition(glm::vec3 pos)
{
	set_position(ToGodot(pos));
}

void EntityPrefabScript::SetName(const ComponentName &name)
{
	Label3D *label = Object::cast_to<Label3D>(find_child("Label3D"));
	label->set_text(String::utf8(name.name.c_str()));
}

void EntityPrefabScript::SetModel(const ComponentModelName &model)
{
	if (currentModel == model) {
		return;
	}
	currentModel = model;

	while (nodeContainingModel->get_child_count() > 0) {
		Node *n = nodeContainingModel->get_child(0);
		nodeContainingModel->remove_child(n);
		n->queue_free();
	}

	if (model.modelName != "") {
		std::string path = std::string("res://assets/") + model.modelName;

		ResourceLoader *rl = ResourceLoader::get_singleton();
		Ref<PackedScene> scene = rl->load(path.c_str(), "PackedScene");
		if (scene.is_null() == false && scene.is_valid()) {
			Node *node = scene->instantiate();
			nodeContainingModel->add_child(node);

			if (IsPlayer()) {
				// TODO: Replace hiding players own model with something better
				// (to hide only animated mesh?)
				if (Node3D *n = Object::cast_to<Node3D>(node)) {
					n->hide();
				}
			}

			animationTree =
				(AnimationTree *)(node->find_child("AnimationTree"));
		} else {
			LOG_INFO("Failed to load scene: `%s`", model.modelName.c_str());
		}
	}
}

bool EntityPrefabScript::IsPlayer() const
{
	return localEntityId == frontend->client->localPlayerEntityId;
}

int64_t EntityPrefabScript::GetLocalEntityId() const { return localEntityId; }

Vector3 EntityPrefabScript::GetRotation() const
{
	auto state = frontend->client->realm->GetComponent<ComponentMovementState>(
		localEntityId);
	if (state) {
		return ToGodot(state->rot);
	}
	LOG_ERROR("state component should exist");
	return {};
}

Vector3 EntityPrefabScript::GetPosition() const
{
	auto state = frontend->client->realm->GetComponent<ComponentMovementState>(
		localEntityId);
	if (state) {
		return ToGodot(state->pos);
	}
	LOG_ERROR("state component should exist");
	return {};
}

Vector3 EntityPrefabScript::GetVelocity() const
{
	auto state = frontend->client->realm->GetComponent<ComponentMovementState>(
		localEntityId);
	if (state) {
		return ToGodot(state->vel);
	}
	LOG_ERROR("state component should exist");
	return {};
}

bool EntityPrefabScript::GetOnGround() const
{
	auto state = frontend->client->realm->GetComponent<ComponentMovementState>(
		localEntityId);
	if (state) {
		return state->onGround;
	}
	LOG_ERROR("state component should exist");
	return true;
}

AnimationTree *EntityPrefabScript::GetAnimationTree() { return animationTree; }

EntityPrefabScript *EntityPrefabScript::CreateNew()
{
	static Ref<PackedScene> prefab;
	if (prefab == nullptr) {
		auto x = ResourceLoader::get_singleton()->load(
			"res://prefabs/MovingEntity.tscn", "PackedScene");
		prefab = x;
	}
	auto ret = (EntityPrefabScript *)(prefab->instantiate());
	return ret;
}
