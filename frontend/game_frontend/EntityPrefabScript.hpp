#pragma once

#include <godot_cpp/variant/utility_functions.hpp>
#include <godot_cpp/godot.hpp>
#include <godot_cpp/core/class_db.hpp>

#include <gdextension_interface.h>
#include <godot_cpp/core/defs.hpp>
#include <godot_cpp/godot.hpp>
#include <godot_cpp/core/engine_ptrcall.hpp>
#include <godot_cpp/classes/scene_tree.hpp>
#include <godot_cpp/classes/mesh_instance3d.hpp>

#include <godot_cpp/classes/node2d.hpp>
#include <godot_cpp/variant/packed_byte_array.hpp>
#include <godot_cpp/classes/rigid_body3d.hpp>
#include <godot_cpp/classes/collision_shape3d.hpp>
#include <godot_cpp/classes/capsule_shape3d.hpp>

#include <glm/glm.hpp>

#include "../../common/include/EntityData.hpp"

using namespace godot;

class GameFrontend;

class EntityPrefabScript : public Node
{
	GDCLASS(EntityPrefabScript, Node)
public: // Godot bound functions
	EntityPrefabScript();
	static void _bind_methods();

	// 	void _enter_tree() override;
	// 	void _exit_tree() override;
	void _ready() override;
	void _process(double dt) override;
	// 	void _my_internal_process(double dt);
	// 	void _physics_process(double dt) override;

	void Init(uint64_t localEntityId);

public:
	void SetModel(const EntityModelName &model);
	void SetPosition(glm::vec3 pos);
	void SetRotation(glm::vec3 rot);

private: // Godot callbacks
	inline GameFrontend *GetGameFrontend() { return gameFrontend; }

	bool IsPlayer() const;
	int64_t GetLocalEntityId() const;
	Vector3 GetRotation() const;
	Vector3 GetPosition() const;
	Vector3 GetVelocity() const;

public: // variables
	uint64_t localEntityId = 0;
	GameFrontend *gameFrontend = nullptr;

	MeshInstance3D *meshInstance = nullptr;

public:
	static EntityPrefabScript *CreateNew();
};