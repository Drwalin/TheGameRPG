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
		REGISTER_PROPERTY_RESOURCE(PrefabServerStaticMesh,
								   graphic_Mesh_or_PackedScene,
								   Variant::Type::OBJECT, "Resource", "scene");
		REGISTER_PROPERTY_RESOURCE(PrefabServerStaticMesh, collision_mesh,
								   Variant::Type::OBJECT, "Mesh", "mesh");
	}

	void _ready() override
	{
		PrefabServerBase::_ready();
		col = nullptr;
		graph = nullptr;
	}

	void _process(double dt) override
	{
		PrefabServerBase::_process(dt);
		if ((col != nullptr) != GameEditorConfig::render_collision) {
			RecreateCollision();
		}

		if ((graph != nullptr) != GameEditorConfig::render_graphic) {
			RecreateGraphic();
		}
	}

	void RecreateCollision()
	{
		RecreateResourceRenderer(&col, &collision_mesh,
								 GameEditorConfig::render_collision);
	}

	void RecreateGraphic()
	{
		RecreateResourceRenderer(&graph, &graphic_Mesh_or_PackedScene,
								 GameEditorConfig::render_graphic);
	}

	virtual void Serialize(uint16_t higherLevelComponentsCount,
						   icon7::ByteWriter &writer) override;

public: // variables
	Ref<Resource> graphic_Mesh_or_PackedScene;
	inline Ref<Resource> get_graphic_Mesh_or_PackedScene()
	{
		return graphic_Mesh_or_PackedScene;
	}
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
