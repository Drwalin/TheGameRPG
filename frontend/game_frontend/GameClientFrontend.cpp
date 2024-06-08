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
void GameClientFrontend::OnEntityAdd(uint64_t localId)
{
	LOG_ERROR("This should be removed in favor of ecs::observer");
}
void GameClientFrontend::OnEntityRemove(uint64_t localId)
{
	// TODO; remove OnEntityRemove
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
	LOG_ERROR("Remove OnEntityShape() this in favor of ecs::observer");
	// TODO: ???
}
void GameClientFrontend::OnEntityModel(uint64_t localId,
									   const EntityModelName &model)
{
	LOG_ERROR("Remove OnEntityModel() in favor of ecs::observer");

	if (realm->HasComponent<EntityGodotNode>(localId) == false) {
		LOG_INFO("OnEntityModel null godot node");
		return;
	}
	/*
	EntityPrefabScript *node =
		realm->GetComponent<EntityGodotNode>(localId)->node;
	node->SetModel(model);
	*/
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
			EntityGodotNode *node =
				(EntityGodotNode *)entity.get<EntityGodotNode>();
			if (node) {
				if (node->node) {
					node->node->SetName(name);
				}
			}
		});

	realm->RegisterObserver(
		flecs::OnAdd,
		[this](flecs::entity entity, const EntityModelName &modelName,
			   const EntityMovementState &movementState) {
			if (entity.has<EntityGodotNode>() == false) {
				LOG_INFO("On entity add model and movementstate and godot node "
						 "null for: %lu",
						 entity.id());
				EntityPrefabScript *node = EntityPrefabScript::CreateNew();
				node->Init(entity.id());
				node->frontend = this->frontend;
				frontend->GetNodeToAddEntities()->add_child(node);
				realm->SetComponent(entity.id(), EntityGodotNode{node});
				const EntityName *name =
					realm->GetComponent<EntityName>(entity.id());
				if (name) {
					node->SetName(*name);
				}
			}
		});

	realm->RegisterObserver(flecs::OnSet, [this](flecs::entity entity,
												 const EntityModelName &model) {
		auto transform = entity.get<EntityStaticTransform>();
		if (transform) {
			LOG_INFO("Setting EntityGodotNode to static");
			EntityStaticGodotNode node;
			if (auto n = entity.get<EntityStaticGodotNode>()) {
				node = *n;
			}
			if (node.node == nullptr) {
				node.node = EntityStaticPrefabScript::CreateNew();
				frontend->GetNodeToAddStaticMap()->add_child(node.node);
			}
			node.node->Init(entity.id(), model, *transform);
			entity.set<EntityStaticGodotNode>(node);
			return;
		}
		EntityGodotNode *node =
			(EntityGodotNode *)entity.get<EntityGodotNode>();
		if (node) {
			LOG_INFO("Setting EntityGodotNode to moving %lu", entity.id());
			node->node->SetModel(model);
		}
	});

	realm->RegisterObserver(
		flecs::OnSet,
		+[](flecs::entity entity, const EntityStaticTransform &transform) {
			if (auto node = entity.get<EntityStaticGodotNode>()) {
				node->node->SetTransform(transform);
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
