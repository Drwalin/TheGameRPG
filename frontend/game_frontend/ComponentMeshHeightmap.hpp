#pragma once

#include <span>

#include <godot_cpp/variant/utility_functions.hpp>
#include <godot_cpp/godot.hpp>
#include <godot_cpp/core/class_db.hpp>

#include <gdextension_interface.h>
#include <godot_cpp/core/defs.hpp>
#include <godot_cpp/godot.hpp>
#include <godot_cpp/core/engine_ptrcall.hpp>
#include <godot_cpp/classes/scene_tree.hpp>
#include <godot_cpp/classes/mesh_instance3d.hpp>
#include <godot_cpp/classes/animation_tree.hpp>
#include <godot_cpp/classes/texture_layered.hpp>
#include <godot_cpp/classes/shader_material.hpp>
#include <godot_cpp/classes/array_mesh.hpp>
#include <godot_cpp/classes/image.hpp>
#include <godot_cpp/classes/image_texture.hpp>
#include <godot_cpp/classes/image_texture_layered.hpp>
#include <godot_cpp/variant/packed_byte_array.hpp>
#include <godot_cpp/variant/packed_float32_array.hpp>

#include <godot_cpp/classes/node2d.hpp>
#include <godot_cpp/variant/packed_byte_array.hpp>
#include <godot_cpp/classes/rigid_body3d.hpp>
#include <godot_cpp/classes/collision_shape3d.hpp>
#include <godot_cpp/classes/capsule_shape3d.hpp>
#include <godot_cpp/classes/mesh_instance3d.hpp>

#include "../../thirdparty/Collision3D/SpatialPartitioning/glm/glm/ext/vector_float3.hpp"

using namespace godot;

class GameFrontend;

class ComponentMeshHeightmap : public MeshInstance3D
{
	GDCLASS(ComponentMeshHeightmap, MeshInstance3D)
public: // Godot bound functions
	ComponentMeshHeightmap();
	~ComponentMeshHeightmap();
	static void _bind_methods();

	void _ready() override;
	void _process(double dt) override;

public:
	void Update(int width, int depth, glm::vec3 scale,
				std::span<uint8_t> material, std::span<float> heights);

public: // variables
	uint64_t localEntityId = 0;
	GameFrontend *frontend = nullptr;
	
	Ref<TextureLayered> materialArray;
	Ref<TextureLayered> get_materialArray() { return materialArray; }
	void set_materialArray(Ref<TextureLayered> v) { materialArray = v; }

	Ref<ShaderMaterial> material;
	Ref<Shader> shader;
};
