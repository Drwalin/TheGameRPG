#include "godot_cpp/classes/engine.hpp"
#include "godot_cpp/classes/engine_debugger.hpp"
#include "godot_cpp/classes/resource_loader.hpp"
#include <godot_cpp/classes/node2d.hpp>
#include <godot_cpp/classes/window.hpp>
#include <godot_cpp/classes/packed_scene.hpp>
#include "godot_cpp/variant/utility_functions.hpp"

#include "GameFrontend.hpp"

#include "GodotGlm.hpp"

#include "EntityPrefabScript.hpp"

#define METHOD_NO_ARGS(CLASS, NAME)                                            \
	ClassDB::bind_method(D_METHOD(#NAME), &CLASS::NAME);

#define METHOD_ARGS(CLASS, NAME, ...)                                          \
	ClassDB::bind_method(D_METHOD(#NAME, __VA_ARGS__), &CLASS::NAME);

EntityPrefabScript::EntityPrefabScript() {}

void EntityPrefabScript::_bind_methods()
{
	METHOD_NO_ARGS(EntityPrefabScript, GetGameFrontend);
	
	METHOD_NO_ARGS(EntityPrefabScript, IsPlayer);
	METHOD_NO_ARGS(EntityPrefabScript, GetLocalEntityId);
	METHOD_NO_ARGS(EntityPrefabScript, GetRotation);
	METHOD_NO_ARGS(EntityPrefabScript, GetPosition);
	METHOD_NO_ARGS(EntityPrefabScript, GetVelocity);
}

void EntityPrefabScript::_ready()
{
	if (Engine::get_singleton()->is_editor_hint()) {
		return;
	}

	gameFrontend =
		(GameFrontend *)(get_tree()->get_root()->get_node_or_null("gameFrontend"));

	if (Engine::get_singleton()->is_editor_hint()) {
		return;
	}
	meshInstance = (MeshInstance3D *)(get_node<Node>("MeshInstance3D"));
	UtilityFunctions::print("physics body is not null, setting meshInstance(",
							(uint64_t)(meshInstance), ")");
}

void EntityPrefabScript::_process(double dt)
{
	if (Engine::get_singleton()->is_editor_hint()) {
		return;
	}

	flecs::entity entity =
		gameFrontend->gameClientFrontend->realm.Entity(localEntityId);
	if (entity.is_alive() == false) {
		return;
	}
	auto state = entity.get<EntityMovementState>();
	if (state) {
		SetRotation(state->rot);
		SetPosition(state->pos);
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
	meshInstance->set_rotation(ToGodot(rot));
}

void EntityPrefabScript::SetPosition(glm::vec3 pos)
{
	meshInstance->set_position(ToGodot(pos));
}

void EntityPrefabScript::SetModel(const EntityModelName &model)
{
	// TODO: implement
}

bool EntityPrefabScript::IsPlayer() const
{
	return localEntityId ==
		   gameFrontend->gameClientFrontend->localPlayerEntityId;
}

int64_t EntityPrefabScript::GetLocalEntityId() const { return localEntityId; }

Vector3 EntityPrefabScript::GetRotation() const
{
	auto state = gameFrontend->gameClientFrontend->realm
					 .GetComponent<EntityMovementState>(localEntityId);
	if (state) {
		return ToGodot(state->rot);
	}
	DEBUG("state component should exist");
	return {};
}

Vector3 EntityPrefabScript::GetPosition() const
{
	auto state = gameFrontend->gameClientFrontend->realm
					 .GetComponent<EntityMovementState>(localEntityId);
	if (state) {
		return ToGodot(state->pos);
	}
	DEBUG("state component should exist");
	return {};
}

Vector3 EntityPrefabScript::GetVelocity() const
{
	auto state = gameFrontend->gameClientFrontend->realm
					 .GetComponent<EntityMovementState>(localEntityId);
	if (state) {
		return ToGodot(state->vel);
	}
	DEBUG("state component should exist");
	return {};
}

EntityPrefabScript *EntityPrefabScript::CreateNew()
{
	static Ref<PackedScene> prefab;
	if (prefab == nullptr) {
		auto x = ResourceLoader::get_singleton()->load(
			"res://prefabs/MovingEntity.tscn", "PackedScene");
		prefab = x;
	}
	return (EntityPrefabScript *)(prefab->instantiate());
}
