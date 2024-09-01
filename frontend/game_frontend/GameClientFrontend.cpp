#include <godot_cpp/classes/resource_loader.hpp>
#include <godot_cpp/classes/mesh.hpp>
#include <godot_cpp/classes/packed_scene.hpp>
#include <godot_cpp/classes/file_access.hpp>
#include <godot_cpp/classes/label3d.hpp>
#include <godot_cpp/classes/animation_player.hpp>

#include <icon7/Debug.hpp>

#include "../../client/include/RealmClient.hpp"
#include "../../common/include/CollisionLoader.hpp"

#include "GodotGlm.hpp"
#include "EntityPrefabScript.hpp"
#include "GameFrontend.hpp"
#include "EntityComponentsFrontend.hpp"
#include "GameFrontend.hpp"
#include "NodeRemoverAfterTimer.hpp"
#include "EntityStaticPrefabScript.hpp"

#include "GameClientFrontend.hpp"

GameClientFrontend *GameClientFrontend::singleton = nullptr;

GameClientFrontend::GameClientFrontend(GameFrontend *frontend)
{
	GameClientFrontend::singleton = this;
	this->frontend = frontend;
}

GameClientFrontend::~GameClientFrontend() {}

int RegisterFrontendEntityComponents(flecs::world &ecs);

void GameClientFrontend::Init()
{
	BindRpc();
	RunNetworkLoopAsync();
	GameClientFrontend::RegisterObservers();
	RegisterFrontendEntityComponents(this->realm->ecs);
}

bool GameClientFrontend::GetCollisionShape(std::string collisionShapeName,
										   TerrainCollisionData *data)
{
	String path = (std::string("res://assets/") + collisionShapeName).c_str();
	PackedByteArray bytes = FileAccess::get_file_as_bytes(path);
	if (bytes.size() == 0) {
		return false;
	}
	CollisionLoader loader;
	loader.LoadOBJ(bytes.ptr(), bytes.size());
	std::swap(loader.collisionData, *data);
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

void GameClientFrontend::PlayDeathAndDestroyEntity_virtual(
	ComponentModelName modelName, ComponentMovementState state,
	ComponentName name)
{
	if (modelName.modelName != "") {
		std::string path = std::string("res://assets/") + modelName.modelName;

		ResourceLoader *rl = ResourceLoader::get_singleton();
		Ref<PackedScene> scene = rl->load(path.c_str(), "PackedScene");
		if (scene.is_null() == false && scene.is_valid()) {
			Node *node = scene->instantiate();
			if (node == nullptr) {
				LOG_ERROR("Cannot instantiate '%s'", path.c_str());
				return;
			}
			Node3D *node3d = Object::cast_to<Node3D>(node);
			if (node3d == nullptr) {
				LOG_ERROR("Cannot convert scene to godot::Node3D to play death "
						  "animation. '%s'",
						  path.c_str());
				delete node;
				return;
			}

			AnimationPlayer *animationPlayer =
				(AnimationPlayer *)(node->find_child("AnimationPlayer"));
			if (animationPlayer == nullptr) {
				LOG_ERROR("Scene '%s' does not have AnimationPlayer",
						  path.c_str());
				delete node;
				return;
			}

			AnimationTree *animationTree =
				(AnimationTree *)(node->find_child("AnimationTree"));
			if (animationTree != nullptr) {
				node->remove_child(animationTree);
				animationTree->queue_free();
			}

			frontend->entitiesContainer->add_child(node3d);

			LOG_INFO("Set transform: %.2f %.2f %.2f     r: %.2f", state.pos.x,
					 state.pos.y, state.pos.z, state.rot.y);

			node3d->set_rotation(ToGodot({0, state.rot.y, 0}));
			node3d->set_position(ToGodot(state.pos));
			Label3D *label = new Label3D();
			label->set_text(String::utf8(name.name.c_str()));
			node3d->add_child(label);
			label->set_position({0, 2, 0});

			NodeRemoverAfterTimer *rem = new NodeRemoverAfterTimer();
			rem->remainingSeconds = 30.f;
			node->add_child(rem);
			rem->set_owner(node);

			PackedStringArray allAnimations =
				animationPlayer->get_animation_list();
			PackedStringArray deathAnimations;
			LOG_INFO("Animations:");
			for (int i = 0; i < allAnimations.size(); ++i) {
				String s = allAnimations[i];
				if (s.to_lower().contains("death") ||
					s.to_lower().contains("dying")) {
					deathAnimations.append(s);
				}
				LOG_INFO("            Anim: %s", s.utf8().ptr());
			}
			if (deathAnimations.size() > 0) {
				int id = rand() % deathAnimations.size();
				animationPlayer->play(deathAnimations[id]);
				LOG_INFO("Play animation: %s",
						 deathAnimations[id].utf8().ptr());
			} else {
				LOG_INFO("No death animations");
				node->queue_free();
			}
		} else {
			LOG_INFO("Failed to load scene: `%s`", modelName.modelName.c_str());
		}
	}
}

void GameClientFrontend::PlayAnimation_virtual(uint64_t serverId,
											   ComponentModelName modelName,
											   ComponentMovementState state,
											   std::string currentAnimation,
											   int64_t animationStartTick)
{
}

void GameClientFrontend::PlayFX(ComponentModelName modelName,
								ComponentStaticTransform transform,
								int64_t timeStartPlaying,
								uint64_t attachToEntityId)
{
}
