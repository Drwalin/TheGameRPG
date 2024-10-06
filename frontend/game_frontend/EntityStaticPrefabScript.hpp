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

#include "../../thirdparty/glm/glm/ext/vector_float3.hpp"

#include "../../common/include/EntityComponents.hpp"

using namespace godot;

class GameFrontend;

class EntityStaticPrefabScript : public Node3D
{
	GDCLASS(EntityStaticPrefabScript, Node3D)
public: // Godot bound functions
	EntityStaticPrefabScript();
	static void _bind_methods();

	void _ready() override;
	void _process(double dt) override;

	void Init(uint64_t localEntityId, const ComponentModelName &model,
			  ComponentStaticTransform transform);

	void SetTransform(const ComponentStaticTransform &transform);

public: // variables
	uint64_t localEntityId = 0;

public:
	static EntityStaticPrefabScript *CreateNew();
};
