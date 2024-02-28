#include "godot_cpp/classes/static_body3d.hpp"
#include "godot_cpp/classes/camera3d.hpp"
#include "godot_cpp/classes/input.hpp"
#include "godot_cpp/classes/engine.hpp"
#include "godot_cpp/classes/engine_debugger.hpp"

#include "../include/GodotGlm.hpp"
#include "../include/EntityPrefabScript.hpp"
#include "../include/ClientConnection.hpp"
#include "godot_cpp/variant/utility_functions.hpp"

EntityPrefabScript::EntityPrefabScript()
{
	DEBUG("construct EntityPrefabScript: %lu", this);
}

void EntityPrefabScript::_bind_methods()
{
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
	
// 		rigidBody = dynamic_cast<godot::RigidBody3D *>(physicsBody);
// 		staticBody = dynamic_cast<godot::StaticBody3D *>(physicsBody);
	
	if (physicsBody) {
	collisionShape = dynamic_cast<godot::CollisionShape3D *>(physicsBody->find_child("CollisionShape"));
	}
	
	DEBUG("physics body: %p", physicsBody);
}

void EntityPrefabScript::_process(double dt)
{
	if (godot::Engine::get_singleton()->is_editor_hint()) {
		return;
	}
	
	if (entityId == 0) {
		return;
	}
	if (lastUpdatedTime==0) {
		if (requestedEntity==false && entities->GetEntity(entityId) == nullptr) {
			clientConnection->RequestEntityData(entityId);
			requestedEntity = true;
		}
		return;
	}
	
	// TODO: do update here
	auto entity = entities->GetEntity(entityId);
	if (entity) {
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
// 		DEBUG("REACHED HERE!");
		const auto state = entity->movementState;
		if (physicsBody) {
			uint64_t currentTime = entities->timer.currentTick;
			float dt = ((int64_t)currentTime - (int64_t)state.timestamp)*0.001f;
			DEBUG("dt: %f <- %li = %li - %li", dt, currentTime - state.timestamp, currentTime, state.timestamp);
			if (dt < 0) {
				return;
			}
			physicsBody->set_position(ToGodot(state.pos));
			glm::vec3 acc = clientConnection->gravity;
			glm::vec3 move = state.vel * dt + acc*(dt*dt*0.5f);
// 			rigidBody->set_linear_velocity(ToGodot(state.vel + acc*dt));
// 			rigidBody->set_linear_velocity({0,0,0});
			physicsBody->set_position(ToGodot(state.pos));
			physicsBody->move_and_collide(ToGodot(move));
			
			godot::UtilityFunctions::print("Received update position: ", physicsBody->get_position());
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
