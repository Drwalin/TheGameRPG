#include <godot_cpp/classes/resource_loader.hpp>
#include <godot_cpp/classes/mesh.hpp>
#include <godot_cpp/classes/packed_scene.hpp>

#include "../../client/include/RealmClient.hpp"

#include "EntityPrefabScript.hpp"
#include "GameFrontend.hpp"
#include "EntityComponentsFrontend.hpp"
#include "GameFrontend.hpp"
#include "GodotGlm.hpp"
#include "EntityPrefabScript.hpp"
#include "EntityStaticPrefabScript.hpp"

#include "GameClientFrontend.hpp"

GameClientFrontend::GameClientFrontend(GameFrontend *frontend)
{
	this->frontend = frontend;
}

GameClientFrontend::~GameClientFrontend() {}

int RegisterFrontendEntityComponents(flecs::world &ecs);

void GameClientFrontend::Init()
{
	flecs::world ecs;
	RegisterFrontendEntityComponents(ecs);
	BindRpc();
	RunNetworkLoopAsync();
	GameClientFrontend::RegisterObservers();
	RegisterFrontendEntityComponents(this->realm->ecs);
}

bool GameClientFrontend::GetCollisionShape(std::string collisionShapeName,
										   TerrainCollisionData *data)
{
	ResourceLoader *rl = ResourceLoader::get_singleton();
	Ref<Mesh> mesh = rl->load(
		(std::string("res://assets/") + collisionShapeName).c_str(), "Mesh");
	if (mesh.is_null()) {
		return false;
	}
	const auto arr = mesh->get_faces();
	const uint32_t size = arr.size() - (arr.size() % 3);
	for (uint32_t i = 0; i < size; ++i) {
		data->vertices.push_back(ToGlm(arr[i]));
		data->indices.push_back(i);
	}
	if (data->indices.size() >= 3 && data->vertices.size() >= 3) {
		return true;
	}
	return false;
}

void GameClientFrontend::OnEnterRealm(const std::string &realmName) {}

void GameClientFrontend::OnSetPlayerId(uint64_t localId)
{
	if (localId == 0) {
		OnPlayerIdUnset();
		return;
	}
	// TODO: ???
}

void GameClientFrontend::OnPlayerIdUnset()
{
	// TODO: ???
}

void GameClientFrontend::LoginFailed(std::string reason)
{
	frontend->OnLoginFailed(reason);
}

void GameClientFrontend::LoginSuccessfull() { frontend->OnLoginSuccessfull(); }

void GameClientFrontend::RunOneEpoch()
{
	if (frontend->IsDisconnected()) {
		DisconnectRealmPeer();
	}
	GameClient::RunOneEpoch();
}

void GameClientFrontend::RegisterObservers()
{
	realm->RegisterObserver(
		flecs::OnSet, +[](flecs::entity entity, const ComponentName &name) {
			ComponentGodotNode *node =
				(ComponentGodotNode *)entity.get<ComponentGodotNode>();
			if (node) {
				if (node->node) {
					node->node->SetName(name);
				}
			}
		});

	realm->RegisterObserver(
		flecs::OnAdd,
		[this](flecs::entity entity, const ComponentModelName &modelName,
			   const ComponentMovementState &movementState) {
			if (entity.has<ComponentGodotNode>() == false) {
				EntityPrefabScript *node = EntityPrefabScript::CreateNew();
				node->Init(entity.id());
				node->frontend = frontend;
				frontend->GetNodeToAddEntities()->add_child(node);
				realm->SetComponent(entity.id(), ComponentGodotNode{node});
				const ComponentName *name =
					realm->GetComponent<ComponentName>(entity.id());
				if (name) {
					node->SetName(*name);
				}
			}
		});

	realm->RegisterObserver(
		flecs::OnSet,
		[this](flecs::entity entity, const ComponentModelName &model) {
			auto transform = entity.get<ComponentStaticTransform>();
			if (transform) {
				ComponentStaticGodotNode node;
				if (auto n = entity.get<ComponentStaticGodotNode>()) {
					node = *n;
				}
				if (node.node == nullptr) {
					node.node = EntityStaticPrefabScript::CreateNew();
					frontend->GetNodeToAddStaticMap()->add_child(node.node);
				}
				node.node->Init(entity.id(), model, *transform);
				entity.set<ComponentStaticGodotNode>(node);
				return;
			}
			ComponentGodotNode *node =
				(ComponentGodotNode *)entity.get<ComponentGodotNode>();
			if (node) {
				node->node->SetModel(model);
			}
		});

	realm->RegisterObserver(
		flecs::OnSet,
		+[](flecs::entity entity, const ComponentStaticTransform &transform) {
			if (auto node = entity.get<ComponentStaticGodotNode>()) {
				node->node->SetTransform(transform);
			}
		});

	realm->RegisterObserver(
		flecs::OnRemove,
		+[](flecs::entity entity, ComponentStaticGodotNode &node) {
			if (node.node) {
				node.node->localEntityId = 0;
				if (node.node->get_parent()) {
					node.node->get_parent()->remove_child(node.node);
				}
				node.node->queue_free();
				node.node = nullptr;
			}
		});

	realm->RegisterObserver(
		flecs::OnRemove, +[](flecs::entity entity, ComponentGodotNode &node) {
			if (node.node) {
				node.node->localEntityId = 0;
				if (node.node->get_parent()) {
					node.node->get_parent()->remove_child(node.node);
				}
				node.node->queue_free();
				node.node = nullptr;
			}
		});

	// TODO: add ecs::observer(OnSet, ComponentShape)
}
