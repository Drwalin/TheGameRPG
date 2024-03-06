#include "Rpc.hpp"
#include "godot_cpp/classes/static_body3d.hpp"
#include "godot_cpp/classes/camera3d.hpp"
#include "godot_cpp/classes/input.hpp"
#include "godot_cpp/classes/engine.hpp"
#include "godot_cpp/classes/engine_debugger.hpp"

#include "../include/GodotGlm.hpp"
#include "../include/EntityPrefabScript.hpp"
#include "../include/ClientConnection.hpp"
#include "godot_cpp/variant/utility_functions.hpp"
#include "icon7/Flags.hpp"

EntityPrefabScript::EntityPrefabScript()
{
	DEBUG("construct EntityPrefabScript: %lu", this);
}

void EntityPrefabScript::_bind_methods()
{
	METHOD_NO_ARGS(EntityPrefabScript, IsPlayer);
	METHOD_NO_ARGS(EntityPrefabScript, GetHost);
	METHOD_NO_ARGS(EntityPrefabScript, GetClient);
	METHOD_NO_ARGS(EntityPrefabScript, GetClientConnection);
	
	METHOD_NO_ARGS(EntityPrefabScript, GetMovementSpeed);
	METHOD_NO_ARGS(EntityPrefabScript, Jump);
	
	METHOD_ARGS(EntityPrefabScript, AddInputMovement, "movement");
	METHOD_ARGS(EntityPrefabScript, SetRotation, "euler");
	METHOD_NO_ARGS(EntityPrefabScript, GetRotation);
	
	METHOD_ARGS(EntityPrefabScript, _my_internal_process, "delta");
	
	METHOD_NO_ARGS(EntityPrefabScript, GetCamera);
	METHOD_ARGS(EntityPrefabScript, SetCamera, "camera");
	godot::ClassDB::add_property("EntityPrefabScript",
			godot::PropertyInfo(godot::Variant::OBJECT, "playerCamera"),
			"SetCamera", "GetCamera");
}

void EntityPrefabScript::_enter_tree()
{
	if (godot::Engine::get_singleton()->is_editor_hint()) {
		return;
	}
	
	rpcHost = this->get_node<RpcHost>("/root/rpcHost");
	rpc = &rpcHost->rpc;
	clientConnection = this->get_node<ClientConnection>("/root/connectionClient");
	entities = &clientConnection->entities;
}

void EntityPrefabScript::_exit_tree()
{
	if (godot::Engine::get_singleton()->is_editor_hint()) {
		return;
	}
	
	auto entity = entities->GetEntity(entityId);
	if (entity) {
		entity->godotNode = nullptr;
	}
	
	if (playerCamera != nullptr) {
		physicsBody->remove_child(playerCamera);
		auto root = get_node<godot::Node3D>("/root/SceneRoot");
		root->add_child(playerCamera);
		playerCamera = nullptr;
	}
}

void EntityPrefabScript::_ready()
{
	if (godot::Engine::get_singleton()->is_editor_hint()) {
		return;
	}
	physicsBody = (godot::PhysicsBody3D *)get_node<godot::Node>("PhysicsBody");
	auto c = this->get_children();
	godot::UtilityFunctions::print("Begin ready prefab");
	for (int i=0; i<c.size(); ++i) {
		auto x = c[i];
		auto ob = ((godot::Object *)(c[i]));
		auto n = (godot::Node *)ob;
		godot::UtilityFunctions::print("Ready with child: ", c[i], " of name: `", n->get_name(), "`");
	}
	godot::UtilityFunctions::print("End ready prefab");
	
	if (physicsBody) {
		collisionShape = (godot::CollisionShape3D *)(physicsBody->get_node<godot::Node>("CollisionShape"));
		meshInstance = (godot::MeshInstance3D *)(physicsBody->get_node<godot::MeshInstance3D>("MeshInstance3D"));
	}
	
	DEBUG("physics body: %p", physicsBody);
}

void EntityPrefabScript::_process(double dt)
{
	_my_internal_process(dt);
}

void EntityPrefabScript::_my_internal_process(double dt)
{
	if (godot::Engine::get_singleton()->is_editor_hint()) {
		return;
	}
	
	if (entityId == 0) {
		godot::UtilityFunctions::print("Entity id == 0");
		return;
	}
	if (lastUpdatedTime==0) {
		if (requestedEntity==false && entities->GetEntity(entityId) == nullptr) {
			clientConnection->RequestEntityData(entityId);
			requestedEntity = true;
		}
		godot::UtilityFunctions::print("Last updated time == 0");
		return;
	}
	
	// TODO: do update here
	auto entity = entities->GetEntity(entityId);
	if (entity) {
		if (entityId == clientConnection->playerEntityId) {
			if (playerCamera == nullptr) {
				playerCamera = (godot::Camera3D *)get_node_or_null("/root/SceneRoot/Camera3D");
				if (playerCamera) {
					godot::UtilityFunctions::print("Camera found");
					playerCamera->get_parent_node_3d()->remove_child(playerCamera);
					physicsBody->add_child(playerCamera);
					playerCamera->set_position({0, entity->longState.height-0.2f, 0});
				} else {
					godot::UtilityFunctions::print("No camera found!");
				}
			}
			if (playerCamera) {
				auto rot = entity->movementState.rot;
				playerCamera->set_rotation(ToGodot({rot.x, 0, rot.z}));
			}
		}
	} else {
		godot::UtilityFunctions::print("entity == null");
	}
}

void EntityPrefabScript::_physics_process(double dt)
{
	
	if (godot::Engine::get_singleton()->is_editor_hint()) {
		return;
	}
	
	if (entityId == 0) {
		godot::UtilityFunctions::print("EntityId === 0, for: ", (uint64_t)(void*)this);
		return;
	}
// 	if (lastUpdatedTime==0) {
// 		return;
// 	}
	
	// TODO: do physics update here
	auto entity = entities->GetEntity(entityId);
	if (entity) {
		auto &state = entity->movementState;
		if (physicsBody) {
			uint64_t currentTime = entities->timer.currentTick;
			float dt = ((int64_t)currentTime - (int64_t)state.timestamp)*0.001f;
// 			DEBUG("dt: %f <- %li = %li - %li", dt, currentTime - state.timestamp, currentTime, state.timestamp);
			if (dt <= 0) {
				return;
			}
			
			if (IsPlayer()) {
				const glm::vec3 m = playerInputMovement;
				playerInputMovement = {0,0,0};
				
				physicsBody->set_position(ToGodot(state.pos));
				glm::vec3 acc = clientConnection->gravity;
				glm::vec3 move = state.vel * dt + acc*(dt*dt*0.5f) + m;
				
				glm::vec3 vel = state.vel + acc*dt;
				state.vel = vel;
				
				physicsBody->move_and_collide(ToGodot(move));
				
				glm::vec3 newPos = ToGlm(physicsBody->get_position());
				
// 				if (glm::length(newPos-state.pos-move) > 0.0001) {
// 					
// 				}
				
				if (newPos.y > state.pos.y+move.y && vel.y < 0) {
					state.vel.y = 0;
				}
				
// 				glm::vec3 realVel = (newPos - state.pos) / dt;
				
				state.pos = newPos;
// 				state.vel = realVel;
				
				physicsBody->set_rotation({0, state.rot.y, 0});
				
				if (m != glm::vec3(0,0,0)) {
					auto peer = clientConnection->peer->peer.get();
					rpc->Send(peer, icon7::FLAG_RELIABLE,
							ServerRemoteFunctions::UpdatePlayer,
							currentTime, state.pos, state.vel, state.rot);
				}
				
				state.timestamp = currentTime;
			} else {
				physicsBody->set_position(ToGodot(state.pos));
				glm::vec3 acc = clientConnection->gravity;
				glm::vec3 move = state.vel * dt + acc*(dt*dt*0.5f);
	// 			rigidBody->set_linear_velocity(ToGodot(state.vel + acc*dt));
	// 			rigidBody->set_linear_velocity({0,0,0});
				physicsBody->move_and_collide(ToGodot(move));
				physicsBody->set_rotation({0, state.rot.y, 0});
			}
			
			lastUpdatedTime = currentTime;
		} else {
			godot::UtilityFunctions::print("PhysicsBody3D is null");
		}
	} else {
		godot::UtilityFunctions::print("Entity is null");
	}
	
}

void EntityPrefabScript::Init(uint64_t entityId)
{
	if (entityId) {
		this->entityId = entityId;
		// TODO: notify or something
	}
}

godot::Camera3D *EntityPrefabScript::GetCamera()
{
	return playerCamera;
}

void EntityPrefabScript::SetCamera(godot::Camera3D *cam)
{
	godot::UtilityFunctions::print("Use of EntityPrefabScript::SetCamera is not allowed!");
}

bool EntityPrefabScript::IsPlayer()
{
	return entityId == clientConnection->playerEntityId && entityId != 0;
}

RpcHost *EntityPrefabScript::GetHost()
{
	return rpcHost;
}

RpcClient *EntityPrefabScript::GetClient()
{
	if (IsPlayer()) {
		return clientConnection->peer;
	}
	return nullptr;
}

ClientConnection *EntityPrefabScript::GetClientConnection()
{
	return clientConnection;
}

void EntityPrefabScript::AddInputMovement(const godot::Vector3 &vel)
{
	playerInputMovement += ToGlm(vel);
}

void EntityPrefabScript::SetRotation(const godot::Vector3 &euler)
{
	auto entity = entities->GetEntity(entityId);
	if (entity && physicsBody) {
		entity->movementState.rot = ToGlm(euler);
		auto client = GetClient();
		if (client) {
			auto peer = client->peer.get();
			rpc->Send(peer, icon7::FLAG_RELIABLE, ServerRemoteFunctions::UpdatePlayerRot,
					ToGlm(euler));
		}
	}
}

godot::Vector3 EntityPrefabScript::GetRotation()
{
	auto entity = entities->GetEntity(entityId);
	if (entity) {
		return ToGodot(entity->movementState.rot);
	}
	return {0,0,0};
}

double EntityPrefabScript::GetMovementSpeed()
{
	return 5;
}

void EntityPrefabScript::Jump()
{
	auto entity = entities->GetEntity(entityId);
	if (entity && physicsBody) {
		entity->movementState.vel.y = std::min(3.6f, entity->movementState.vel.y+6.0f);
	}
}
