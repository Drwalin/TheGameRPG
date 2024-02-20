#include <godot_cpp/classes/height_map_shape3d.hpp>
#include <godot_cpp/classes/array_mesh.hpp>
#include <godot_cpp/classes/mesh_instance3d.hpp>
#include <godot_cpp/classes/static_body3d.hpp>
#include <godot_cpp/classes/collision_shape3d.hpp>

#include "godot_cpp/variant/packed_color_array.hpp"
#include "godot_cpp/variant/packed_float32_array.hpp"
#include "godot_cpp/variant/packed_vector3_array.hpp"
#include "icon7/RPCEnvironment.hpp"

#include "../include/ClientConnection.hpp"

#include "../../../server/include/TerrainMap.hpp"
#include "../../../server/include/Entity.hpp"

void ClientConnection::UpdateTerrain(TerrainMap &map)
{
	auto terrainGroupNode = this->get_node_or_null("/root/terrain");
	auto terrainMeshInstance = terrainGroupNode->get_node<godot::MeshInstance3D>("meshInstance");
	auto terrainPhysics = terrainMeshInstance->get_node<godot::StaticBody3D>("physicsBody");
	auto collisionShape = terrainPhysics->get_node<godot::CollisionShape3D>("collisionShape");
	
	{
		godot::PackedFloat32Array heights;
		heights.resize(map.heights.size());
		memcpy(heights.ptrw(), map.heights.data(), map.heights.size()*sizeof(float));
		
		godot::Ref<godot::Shape3D> _shape = collisionShape->get_shape();
		auto shape = (godot::HeightMapShape3D *)_shape.ptr();
		shape->set_map_width(map.width);
		shape->set_map_depth(map.depth);
		shape->set_map_data(heights);
	}
	
	{
		godot::ArrayMesh *mesh = memnew(godot::ArrayMesh());
		godot::Array arrays;
		arrays.resize(godot::Mesh::ARRAY_MAX);
		
		float scale = map.horizontalScale;
		
		godot::PackedVector3Array vertices;
		godot::PackedColorArray colors;
		for (int x=0; x<map.width-1; ++x) {
			for (int y=0; y<map.depth-1; ++y) {
				godot::Color color;
				color = {0.1f, 0.6f * map.GetHeight(x,y) / 100.0f, 0.05f};
				int dx, dy;
				dx=0; dy=0; vertices.append({(x+dx)*scale, map.GetHeight(x+dx, y+dy), (y+dy)*scale}); colors.append(color);
				dx=1; dy=0; vertices.append({(x+dx)*scale, map.GetHeight(x+dx, y+dy), (y+dy)*scale}); colors.append(color);
				dx=1; dy=1; vertices.append({(x+dx)*scale, map.GetHeight(x+dx, y+dy), (y+dy)*scale}); colors.append(color);
				
				color = {0.1f, 0.6f * map.GetHeight(x,y) / 100.0f, 0.05f};
				dx=0; dy=0; vertices.append({(x+dx)*scale, map.GetHeight(x+dx, y+dy), (y+dy)*scale}); colors.append(color);
				dx=1; dy=1; vertices.append({(x+dx)*scale, map.GetHeight(x+dx, y+dy), (y+dy)*scale}); colors.append(color);
				dx=0; dy=1; vertices.append({(x+dx)*scale, map.GetHeight(x+dx, y+dy), (y+dy)*scale}); colors.append(color);
			}
		}
		
		arrays[godot::Mesh::ARRAY_VERTEX] = vertices;
		arrays[godot::Mesh::ARRAY_COLOR] = colors;
		mesh->add_surface_from_arrays(godot::Mesh::PRIMITIVE_TRIANGLES, arrays);
		
		terrainMeshInstance->set_mesh(mesh);
	}
}


