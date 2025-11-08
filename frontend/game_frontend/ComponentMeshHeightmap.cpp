#include <godot_cpp/classes/engine.hpp>
#include <godot_cpp/classes/engine_debugger.hpp>
#include <godot_cpp/classes/resource_loader.hpp>
#include <godot_cpp/classes/node2d.hpp>
#include <godot_cpp/classes/window.hpp>
#include <godot_cpp/classes/packed_scene.hpp>
#include <godot_cpp/classes/label3d.hpp>

#include "GodotGlm.hpp"

#define METHOD_NO_ARGS(CLASS, NAME)                                            \
	ClassDB::bind_method(D_METHOD(#NAME), &CLASS::NAME);

#define METHOD_ARGS(CLASS, NAME, ...)                                          \
	ClassDB::bind_method(D_METHOD(#NAME, __VA_ARGS__), &CLASS::NAME);

#include "ComponentMeshHeightmap.hpp"

ComponentMeshHeightmap::ComponentMeshHeightmap() {}

ComponentMeshHeightmap::~ComponentMeshHeightmap() {}

#define TO_C_STRING_NAME(STR) #STR
#define REGISTER_PROPERTY_RESOURCE(CLASS, NAME, TYPE, RESOURCE_TYPE, SET_NAME) \
	{                                                                          \
		METHOD_NO_ARGS(CLASS, get_##NAME);                                     \
		METHOD_ARGS(CLASS, set_##NAME, SET_NAME);                              \
		ClassDB::add_property(                                                 \
			#CLASS,                                                            \
			PropertyInfo(TYPE, #NAME, PROPERTY_HINT_RESOURCE_TYPE,             \
						 RESOURCE_TYPE),                                       \
			TO_C_STRING_NAME(set_##NAME), TO_C_STRING_NAME(get_##NAME));       \
	}

void ComponentMeshHeightmap::_bind_methods()
{
	REGISTER_PROPERTY_RESOURCE(ComponentMeshHeightmap, materialArray,
							   Variant::Type::OBJECT, "TextureLayered",
							   "materialArray");
}

void ComponentMeshHeightmap::_ready()
{
	if (Engine::get_singleton()->is_editor_hint()) {
		return;
	}

	frontend = (GameFrontend *)(get_tree()->get_root()->get_node_or_null(
		"gameFrontend"));
}

void ComponentMeshHeightmap::_process(double dt)
{
	if (Engine::get_singleton()->is_editor_hint()) {
		return;
	}
}

void ComponentMeshHeightmap::Update(int width, int depth, glm::vec3 scale,
									std::span<uint8_t> material,
									std::span<float> heights)
{
}
