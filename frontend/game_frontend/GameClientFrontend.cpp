#include <godot_cpp/classes/resource_loader.hpp>
#include <godot_cpp/classes/mesh.hpp>
#include <godot_cpp/classes/packed_scene.hpp>

#include "../../client/include/RealmClient.hpp"

#include "EntityPrefabScript.hpp"
#include "GameFrontend.hpp"
#include "EntityDataFrontend.hpp"
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

void GameClientFrontend::Init()
{
	BindRpc();
	this->RunNetworkLoopAsync();
	GameClientFrontend::RegisterObservers();
}

bool GameClientFrontend::GetCollisionShape(std::string collisionShapeName,
										   TerrainCollisionData *data)
{
	ResourceLoader *rl = ResourceLoader::get_singleton();
	Ref<Mesh> mesh = rl->load((std::string("res://assets/map_collision/") +
							   collisionShapeName + ".obj")
								  .c_str(),
							  "Mesh");
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
void GameClientFrontend::OnEnterRealm(const std::string &realmName)
{
	TerrainCollisionData col;

	// Load collision
	ResourceLoader *rl = ResourceLoader::get_singleton();
	Ref<Mesh> mesh = rl->load(
		(std::string("res://assets/map_collision/") + realmName + ".obj")
			.c_str(),
		"Mesh");
	const auto arr = mesh->get_faces();
	const uint32_t size = arr.size() - (arr.size() % 3);
	for (uint32_t i = 0; i < size; ++i) {
		col.vertices.push_back(ToGlm(arr[i]));
		col.indices.push_back(i);
	}
	realm->collisionWorld.LoadStaticCollision(&col, {{}, {}});

	// Load map
	Node *container = frontend->GetNodeToAddStaticMap();
	while (true) {
		TypedArray<Node> oldScenes = container->get_children();
		if (oldScenes.size() == 0) {
			break;
		}
		container->remove_child(Object::cast_to<Node>(oldScenes[0]));
	}
	Ref<PackedScene> scene = rl->load(
		(std::string("res://assets/scenes/") + realmName + ".tscn").c_str(),
		"PackedScene");
	container->add_child(scene->instantiate());
}
void GameClientFrontend::OnEntityAdd(uint64_t localId)
{
	if (realm->HasComponent<EntityGodotNode>(localId) == false) {
		EntityPrefabScript *node = EntityPrefabScript::CreateNew();
		node->Init(localId);
		node->frontend = frontend;
		frontend->GetNodeToAddEntities()->add_child(node);
		realm->SetComponent(localId, EntityGodotNode{node});
		const EntityName *name = realm->GetComponent<EntityName>(localId);
		if (name) {
			node->SetName(*name);
		}
	}
}
void GameClientFrontend::OnEntityRemove(uint64_t localId)
{
	if (realm->HasComponent<EntityGodotNode>(localId) == false) {
		return;
	}
	EntityPrefabScript *node =
		realm->GetComponent<EntityGodotNode>(localId)->node;
	node->localEntityId = 0;
	if (node->get_parent()) {
		node->get_parent()->remove_child(node);
	}
	node->queue_free();
	realm->SetComponent<EntityGodotNode>(localId, EntityGodotNode{nullptr});
	realm->RemoveComponent<EntityGodotNode>(localId);
}
void GameClientFrontend::OnEntityShape(uint64_t localId,
									   const EntityShape &shape)
{
	// TODO: ???
}
void GameClientFrontend::OnEntityModel(uint64_t localId,
									   const EntityModelName &model)
{
	if (realm->HasComponent<EntityGodotNode>(localId) == false) {
		return;
	}
	EntityPrefabScript *node =
		realm->GetComponent<EntityGodotNode>(localId)->node;
	node->SetModel(model);
}
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
		flecs::OnSet, +[](flecs::entity entity, const EntityName &name) {
			EntityGodotNode *en =
				(EntityGodotNode *)entity.get<EntityGodotNode>();
			if (en) {
				if (en->node) {
					en->node->SetName(name);
				}
			}
		});

	realm->RegisterObserver(flecs::OnSet, [this](flecs::entity entity,
												 const EntityModelName &model) {
		auto t = entity.get<EntityStaticTransform>();
		if (t != nullptr) {
			EntityStaticGodotNode *node =
				realm->AccessComponent<EntityStaticGodotNode>(entity);
			if (node->node == nullptr) {
				node->node->Init(entity.id(), model, *t);
			}
		}
	});

	realm->RegisterObserver(
		flecs::OnRemove,
		+[](flecs::entity entity, EntityStaticGodotNode &node) {
			if (node.node) {
				node.node->queue_free();
				node.node = nullptr;
			}
		});

	realm->RegisterObserver(
		flecs::OnRemove, +[](flecs::entity entity, EntityGodotNode &node) {
			if (node.node) {
				node.node->queue_free();
				node.node = nullptr;
			}
		});
}
