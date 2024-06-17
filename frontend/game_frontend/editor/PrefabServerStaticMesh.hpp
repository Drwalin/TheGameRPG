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
		REGISTER_PROPERTY_RESOURCE(PrefabServerStaticMesh, graphic_mesh,
								   Variant::Type::OBJECT, "PackedScene",
								   "scene");
		REGISTER_PROPERTY_RESOURCE(PrefabServerStaticMesh, collision_mesh,
								   Variant::Type::OBJECT, "Mesh", "mesh");
	}

	void _ready() override {
		while (get_child_count() > 0) {
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
			if (graphic_mesh.is_valid() && !graphic_mesh.is_null()) {
				graph = graphic_mesh->instantiate();
				add_child(graph);
			}
		}
	}

	virtual void Serialize(uint16_t higherLevelComponentsCount,
						   icon7::ByteWriter &writer) override;

public: // variables
	Ref<PackedScene> graphic_mesh;
	inline Ref<PackedScene> get_graphic_mesh() { return graphic_mesh; }
	inline void set_graphic_mesh(Ref<PackedScene> v)
	{
		graphic_mesh = v;
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
