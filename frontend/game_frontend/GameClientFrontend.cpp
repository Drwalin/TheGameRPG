#include <godot_cpp/classes/resource_loader.hpp>
#include <godot_cpp/classes/mesh.hpp>
#include <godot_cpp/classes/packed_scene.hpp>
#include <godot_cpp/classes/file_access.hpp>
#include <godot_cpp/classes/label3d.hpp>
#include <godot_cpp/classes/node3d.hpp>
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

void GameClientFrontend::OnEnterRealm(const std::string &realmName)
{
	auto Del = [](decltype(frontend->entitiesContainer->get_children(true)) c) {
		for (int i = 0; i < c.size(); ++i) {
			c[i].call("queue_free");
		}
	};
	Del(frontend->entitiesContainer->get_children(true));
	Del(frontend->staticMapContainer->get_children(true));
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
	ComponentModelName modelName, ComponentLastAuthoritativeMovementState state,
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

			node3d->set_rotation(ToGodot({0, state.oldState.rot.y, 0}));
			node3d->set_position(ToGodot(state.oldState.pos));
			Label3D *label = new Label3D();
			label->set_text(String::utf8(name.name.c_str()));
			node3d->add_child(label);
			label->set_position({0, 2, 0});

			NodeRemoverAfterTimer *rem = Object::cast_to<NodeRemoverAfterTimer>(
				ClassDB::instantiate("NodeRemoverAfterTimer"));
			rem->remainingSeconds = 30.f;
			node->add_child(rem);
			rem->set_owner(node);

			PackedStringArray allAnimations =
				animationPlayer->get_animation_list();
			PackedStringArray deathAnimations;
			for (int i = 0; i < allAnimations.size(); ++i) {
				String s = allAnimations[i];
				if (s.to_lower().contains("death") ||
					s.to_lower().contains("dying")) {
					deathAnimations.append(s);
				}
			}
			if (deathAnimations.size() > 0) {
				int id = rand() % deathAnimations.size();
				animationPlayer->play(deathAnimations[id]);
			} else {
				LOG_INFO("No death animations for '%s'",
						 modelName.modelName.c_str());
				node->queue_free();
			}
		} else {
			LOG_INFO("Failed to load scene: `%s`", modelName.modelName.c_str());
		}
	}
}

void GameClientFrontend::PlayAnimation_virtual(
	uint64_t localId, ComponentModelName modelName,
	ComponentLastAuthoritativeMovementState state, std::string currentAnimation,
	int64_t animationStartTick)
{
	flecs::entity entity = realm->Entity(localId);
	if (!(entity.is_alive() && entity.is_valid())) {
		LOG_INFO("Failed to add animation");
		return;
	}
	ComponentGodotNode *node =
		(ComponentGodotNode *)entity.get<ComponentGodotNode>();
	if (node) {
		if (node->node) {
			node->node->oneShotAnimations.push_back(currentAnimation);
			// TODO: use animationStartTick
		} else {
			LOG_INFO("Failed to add animation");
		}
	} else {
		LOG_INFO("Failed to add animation");
	}
}

void GameClientFrontend::PlayFX(ComponentModelName modelName,
								ComponentStaticTransform transform,
								int64_t timeStartPlaying,
								uint64_t attachToEntityId, int32_t ttlMs)
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

			if (attachToEntityId != 0) {
				auto it =
					mapServerEntityIdToLocalEntityId.find(attachToEntityId);
				if (it != mapServerEntityIdToLocalEntityId.end()) {
					flecs::entity entity = realm->Entity(it->second);
					if (entity.is_valid() && entity.is_alive()) {

						bool has = false;
						if (auto gn = entity.get<ComponentGodotNode>()) {
							if (gn->node) {
								has = true;
								gn->node->add_child(node3d);
							}
						}

						if (auto gsn = entity.get<ComponentStaticGodotNode>()) {
							if (gsn->node) {
								has = true;
								gsn->node->add_child(node3d);
							}
						}

						if (has == false) {
							attachToEntityId = 0;
						}
					} else {
						attachToEntityId = 0;
					}
				} else {
					attachToEntityId = 0;
				}
			}

			if (attachToEntityId == 0) {
				frontend->entitiesContainer->add_child(node3d);
			}

			node3d->set_transform(Transform3D{Basis(ToGodot(transform.rot)),
											  ToGodot(transform.pos)});
			node3d->set_scale(ToGodot(transform.scale));

			if (ttlMs != 0) {

				NodeRemoverAfterTimer *rem =
					Object::cast_to<NodeRemoverAfterTimer>(
						ClassDB::instantiate("NodeRemoverAfterTimer"));
				rem->remainingSeconds = (timeStartPlaying + (int64_t)ttlMs -
										 realm->timer.currentTick) *
										0.001f;
				node->add_child(rem);
				rem->set_owner(node);
			}
		} else {
			LOG_INFO("Failed to load scene: `%s`", modelName.modelName.c_str());
		}
	}
}
