#pragma once

#include "PrefabServerBase.hpp"

namespace editor
{
class PrefabServerStaticMesh : public PrefabServerBase
{
	GDCLASS(PrefabServerStaticMesh, Node3D)
public: // Godot bound functions
	PrefabServerStaticMesh();
	virtual ~PrefabServerStaticMesh();
	static void _bind_methods()
	{
		REGISTER_PROPERTY_RESOURCE(PrefabServerStaticMesh, graphic_Mesh_or_PackedScene,
								   Variant::Type::OBJECT, "Resource",
								   "scene");
		REGISTER_PROPERTY_RESOURCE(PrefabServerStaticMesh, collision_mesh,
								   Variant::Type::OBJECT, "Mesh", "mesh");
	}

	void _ready() override {
		while (get_child_count() > 0) {
			auto child = get_child(0);
			remove_child(child);
			child->queue_free();
		}
		while (get_child_count(true) > 0) {
			auto child = get_child(0, true);
			remove_child(child);
			child->queue_free();
		}
		col = nullptr;
		graph = nullptr;
	}

	void _process(double dt) override
	{
		if ((col != nullptr) != GameEditorConfig::render_collision) {
			RecreateCollision();
		}

		if ((graph != nullptr) != GameEditorConfig::render_graphic) {
			RecreateGraphic();
		}
		
		set_transform(Transform3D{
				Basis(get_basis().get_quaternion()), get_position()});
	}

	void RecreateCollision()
	{
		if (col) {
			remove_child(col);
			col->queue_free();
			col = nullptr;
		}
		if (GameEditorConfig::render_collision) {
			col = new MeshInstance3D();
			col->set_mesh(collision_mesh);
			add_child(col);
			col->set_owner(this);
		}
	}

	void RecreateGraphic()
	{
		if (graph) {
			remove_child(graph);
			graph->queue_free();
			graph = nullptr;
		}
		if (GameEditorConfig::render_graphic) {
			if (graphic_Mesh_or_PackedScene.is_valid() && !graphic_Mesh_or_PackedScene.is_null()) {
				
				Ref<PackedScene> packedScene = graphic_Mesh_or_PackedScene;
				if (packedScene.is_null() == false && packedScene.is_valid()) {
					graph = packedScene->instantiate();
					add_child(graph);
					graph->set_owner(this);
				}
				
				Ref<Mesh> mesh = graphic_Mesh_or_PackedScene;
				if (mesh.is_null() == false && mesh.is_valid()) {
					MeshInstance3D *m = new MeshInstance3D();
					m->set_mesh(mesh);
					graph = m;
					add_child(graph);
					graph->set_owner(this);
				}
			}
		}
	}

	virtual void Serialize(uint16_t higherLevelComponentsCount,
						   icon7::ByteWriter &writer) override;

public: // variables
	Ref<Resource> graphic_Mesh_or_PackedScene;
	inline Ref<Resource> get_graphic_Mesh_or_PackedScene() { return graphic_Mesh_or_PackedScene; }
	inline void set_graphic_Mesh_or_PackedScene(Ref<Resource> v)
	{
		graphic_Mesh_or_PackedScene = Ref<Resource>{};
		
		Ref<PackedScene> packedScene = v;
		if (packedScene.is_null() == false && packedScene.is_valid()) {
			graphic_Mesh_or_PackedScene = v;
		}

		Ref<Mesh> mesh = v;
		if (mesh.is_null() == false && mesh.is_valid()) {
			graphic_Mesh_or_PackedScene = v;
		}
		
		RecreateGraphic();
	}

	Ref<Mesh> collision_mesh;
	inline Ref<Mesh> get_collision_mesh() { return collision_mesh; }
	inline void set_collision_mesh(Ref<Mesh> v)
	{
		collision_mesh = v;
		RecreateCollision();
	}

public:
	MeshInstance3D *col = nullptr;
	Node *graph = nullptr;
};
} // namespace editor
