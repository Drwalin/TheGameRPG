#pragma once

#include <godot_cpp/variant/utility_functions.hpp>
#include <godot_cpp/godot.hpp>
#include <godot_cpp/core/class_db.hpp>

#include <gdextension_interface.h>
#include <godot_cpp/core/defs.hpp>
#include <godot_cpp/godot.hpp>
#include <godot_cpp/core/engine_ptrcall.hpp>
#include <godot_cpp/classes/scene_tree.hpp>

#include <godot_cpp/classes/node2d.hpp>
#include <godot_cpp/variant/packed_byte_array.hpp>
#include <godot_cpp/classes/rigid_body3d.hpp>
#include <godot_cpp/classes/collision_shape3d.hpp>
#include <godot_cpp/classes/capsule_shape3d.hpp>

#include "../../../ICon7-godot-client/include/icon7-godot-client/Connections.hpp"

#include "Rpc.hpp"
#include "Entities.hpp"
#include "godot_cpp/classes/static_body3d.hpp"

#define METHOD_NO_ARGS(CLASS, NAME)                                            \
	godot::ClassDB::bind_method(godot::D_METHOD(#NAME), &CLASS::NAME);

#define METHOD_ARGS(CLASS, NAME, ...)                                          \
	godot::ClassDB::bind_method(godot::D_METHOD(#NAME, __VA_ARGS__),           \
								&CLASS::NAME);

class EntityPrefabScript : public godot::Node
{
	GDCLASS(EntityPrefabScript, Node)
public: // Godot bound functions
	EntityPrefabScript();
	static void _bind_methods();

	void _enter_tree() override;
	void _exit_tree() override;
	void _ready() override;
	void _process(double dt) override;
	void _physics_process(double dt) override;
	
	void Init(uint64_t entityId);
	
private: // Godot callbacks

public: // variables
	uint64_t entityId = 0;
	RpcHost *rpcHost = nullptr;
	icon7::RPCEnvironment *rpc = nullptr;
	class ClientConnection *clientConnection = nullptr;
	uint64_t lastUpdatedTime = 0;
	Entities *entities = nullptr;
	
	godot::PhysicsBody3D *physicsBody = nullptr;
// 	godot::RigidBody3D *rigidBody = nullptr;
// 	godot::StaticBody3D *staticBody = nullptr;
	
	godot::CollisionShape3D *collisionShape = nullptr;
	godot::CapsuleShape3D *capsuleShape = nullptr;
	
	godot::Camera3D *playerCamera = nullptr;
	bool requestedEntity = false;
};
