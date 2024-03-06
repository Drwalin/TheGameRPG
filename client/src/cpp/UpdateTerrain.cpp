#include <godot_cpp/classes/height_map_shape3d.hpp>
#include <godot_cpp/classes/array_mesh.hpp>
#include <godot_cpp/classes/mesh_instance3d.hpp>
#include <godot_cpp/classes/static_body3d.hpp>
#include <godot_cpp/classes/collision_shape3d.hpp>
#include <godot_cpp/classes/surface_tool.hpp>
#include <godot_cpp/classes/standard_material3d.hpp>
#include <godot_cpp/classes/material.hpp>
#include <godot_cpp/classes/resource_saver.hpp>

#include "godot_cpp/variant/packed_color_array.hpp"
#include "godot_cpp/variant/packed_float32_array.hpp"
#include "godot_cpp/variant/packed_vector3_array.hpp"
#include "godot_cpp/variant/utility_functions.hpp"
#include "icon7/RPCEnvironment.hpp"

#include "../include/ClientConnection.hpp"

#include "../../../server/include/TerrainMap.hpp"
#include "../../../server/include/Entity.hpp"

void ClientConnection::UpdateTerrain(TerrainMap &map)
{
	godot::UtilityFunctions::print("Received terrain: ", map.width, " x ", map.depth);
	
	
	auto terrainGroupNode = this->get_node_or_null("/root/SceneRoot/Terrain");
	auto terrainMeshInstance = (godot::MeshInstance3D *)(this->get_node_or_null("/root/SceneRoot/Terrain/MeshInstance3D"));
	auto terrainPhysics = (godot::StaticBody3D *)(this->get_node_or_null("/root/SceneRoot/Terrain/MeshInstance3D/StaticBody3D"));
	auto collisionShape = (godot::CollisionShape3D *)(this->get_node_or_null("/root/SceneRoot/Terrain/MeshInstance3D/StaticBody3D/CollisionShape3D"));
		
	float scale = map.horizontalScale;
	
	{
		godot::PackedFloat32Array heights;
		heights.resize(map.heights.size());
		memcpy(heights.ptrw(), map.heights.data(), map.heights.size()*sizeof(float));
		
// 		godot::Ref<godot::Shape3D> _shape = collisionShape->get_shape();
// 		auto shape = (godot::HeightMapShape3D *)_shape.ptr();
		godot::Ref<godot::HeightMapShape3D> shape = memnew(godot::HeightMapShape3D());
// 		auto shape = (godot::HeightMapShape3D *)_shape.ptr();
		shape->set_map_width(map.width);
		shape->set_map_depth(map.depth);
		shape->set_map_data(heights);
		collisionShape->set_shape(shape);
		collisionShape->scale_object_local({scale, 1, scale});
// 		terrainPhysics->shape_owner_add_shape(
// 				terrainPhysics->create_shape_owner(terrainPhysics), shape);
	}
	
	{
		godot::SurfaceTool surf;
		godot::Ref<godot::StandardMaterial3D> mat = memnew(godot::StandardMaterial3D());
		mat->set_flag(godot::BaseMaterial3D::Flags::FLAG_ALBEDO_FROM_VERTEX_COLOR, true);
		surf.begin(godot::Mesh::PRIMITIVE_TRIANGLES);
		surf.set_material(mat);
		
		float CFX = scale*(map.width*0.5f - 0.5f);
		float CFZ = scale*(map.depth*0.5f - 0.5f);
		
		godot::PackedVector3Array vertices;
		godot::PackedColorArray colors;
		for (int x=0; x<map.width-1; ++x) {
			for (int y=0; y<map.depth-1; ++y) {
				godot::Color color;
				int dx, dy;
				
				{
				color = {0.1f, 0.6f * map.GetHeight(x,y) / 100.0f, 0.05f};
				dx=0; dy=0; godot::Vector3 a{(x+dx)*scale - CFX, map.GetHeight(x+dx, y+dy), (y+dy)*scale - CFX};
				dx=1; dy=0; godot::Vector3 b{(x+dx)*scale - CFX, map.GetHeight(x+dx, y+dy), (y+dy)*scale - CFX};
				dx=1; dy=1; godot::Vector3 c{(x+dx)*scale - CFX, map.GetHeight(x+dx, y+dy), (y+dy)*scale - CFX};
				godot::Vector3 normal = (a-b).cross(c-b).normalized();
				surf.set_color(color); surf.set_normal(normal); surf.add_vertex(a);
				surf.set_color(color); surf.set_normal(normal); surf.add_vertex(b);
				surf.set_color(color); surf.set_normal(normal); surf.add_vertex(c);
				}
				
				{
				color = {0.1f, 0.6f * map.GetHeight(x,y) / 100.0f, 0.05f};
				dx=0; dy=0; godot::Vector3 a{(x+dx)*scale - CFX, map.GetHeight(x+dx, y+dy), (y+dy)*scale - CFZ};
				dx=1; dy=1; godot::Vector3 b{(x+dx)*scale - CFX, map.GetHeight(x+dx, y+dy), (y+dy)*scale - CFZ};
				dx=0; dy=1; godot::Vector3 c{(x+dx)*scale - CFX, map.GetHeight(x+dx, y+dy), (y+dy)*scale - CFZ};
				godot::Vector3 normal = (a-b).cross(c-b).normalized();
				surf.set_color(color); surf.set_normal(normal); surf.add_vertex(a);
				surf.set_color(color); surf.set_normal(normal); surf.add_vertex(b);
				surf.set_color(color); surf.set_normal(normal); surf.add_vertex(c);
				}
			}
		}
		
		auto mesh = surf.commit();
		terrainMeshInstance->set_mesh(mesh);
		
		
// 		godot::ResourceSaver::get_singleton()->save(mesh, "res://heightmap_mesh.tres");
		
		
		
		
		
		
		
		
		
		
		
		
// 		godot::ArrayMesh *mesh = memnew(godot::ArrayMesh());
// 		godot::Array arrays;
// 		arrays.resize(godot::Mesh::ARRAY_MAX);
// 		
// 		float scale = map.horizontalScale;
// 		
// 		godot::PackedVector3Array vertices;
// 		godot::PackedColorArray colors;
// 		for (int x=0; x<map.width-1; ++x) {
// 			for (int y=0; y<map.depth-1; ++y) {
// 				godot::Color color;
// 				color = {0.1f, 0.6f * map.GetHeight(x,y) / 100.0f, 0.05f};
// 				int dx, dy;
// 				dx=0; dy=0; vertices.append({(x+dx)*scale, map.GetHeight(x+dx, y+dy), (y+dy)*scale}); colors.append(color);
// 				dx=1; dy=0; vertices.append({(x+dx)*scale, map.GetHeight(x+dx, y+dy), (y+dy)*scale}); colors.append(color);
// 				dx=1; dy=1; vertices.append({(x+dx)*scale, map.GetHeight(x+dx, y+dy), (y+dy)*scale}); colors.append(color);
// 				
// 				color = {0.1f, 0.6f * map.GetHeight(x,y) / 100.0f, 0.05f};
// 				dx=0; dy=0; vertices.append({(x+dx)*scale, map.GetHeight(x+dx, y+dy), (y+dy)*scale}); colors.append(color);
// 				dx=1; dy=1; vertices.append({(x+dx)*scale, map.GetHeight(x+dx, y+dy), (y+dy)*scale}); colors.append(color);
// 				dx=0; dy=1; vertices.append({(x+dx)*scale, map.GetHeight(x+dx, y+dy), (y+dy)*scale}); colors.append(color);
// 			}
// 		}
// 		
// 		arrays[godot::Mesh::ARRAY_VERTEX] = vertices;
// 		arrays[godot::Mesh::ARRAY_COLOR] = colors;
// 		mesh->add_surface_from_arrays(godot::Mesh::PRIMITIVE_TRIANGLES, arrays);
// 		
// 		terrainMeshInstance->set_mesh(mesh);
	}
}


