#include <godot_cpp/classes/resource_loader.hpp>
#include <godot_cpp/classes/mesh.hpp>

#include "../../client/include/RealmClient.hpp"

#include "EntityPrefabScript.hpp"
#include "GameFrontend.hpp"
#include "EntityDataFrontend.hpp"
#include "GameFrontend.hpp"
#include "GodotGlm.hpp"

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
}

void GameClientFrontend::OnEnterRealm(const std::string &realmName)
{
	TerrainCollisionData col;
	ResourceLoader *rl = ResourceLoader::get_singleton();
	Ref<Resource> res = rl->load((std::string("res://assets/models/") + realmName + ".obj").c_str(), "Mesh");
	Mesh *mesh = Object::cast_to<Mesh>(res.ptr());
	const auto arr = mesh->get_faces();
	const uint32_t size = arr.size() - (arr.size()%3);
	for (uint32_t i=0; i<size; ++i) {
		col.vertices.push_back(ToGlm(arr[i]));
		col.indices.push_back(i);
	}
	realm->collisionWorld.LoadStaticCollision(&col);
}
void GameClientFrontend::OnEntityAdd(uint64_t localId)
{
	if (realm->HasComponent<EntityGodotNode>(localId) == false) {
		EntityPrefabScript *node = EntityPrefabScript::CreateNew();
		node->Init(localId);
		node->frontend = frontend;
		frontend->GetNodeToAddEntities()->add_child(node);
		realm->SetComponent(localId, EntityGodotNode{node});
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
