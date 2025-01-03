#pragma once

#include "PrefabServerBase.hpp"

namespace editor
{
class PrefabServerStaticMesh_Base : public PrefabServerBase
{
	GDCLASS(PrefabServerStaticMesh_Base, PrefabServerBase)
public: // Godot bound functions
	PrefabServerStaticMesh_Base();
	virtual ~PrefabServerStaticMesh_Base();
	static void _bind_methods();

	void _ready() override;
	void _process(double dt) override;

	void RecreateCollision();
	void RecreateGraphic();

	virtual void Serialize(icon7::ByteWriter &writer) override;

	Transform3D GetMergingData(Ref<Resource> *graphicMeshOrPackedScene,
							   Ref<Mesh> *collisionMesh);

public: // variables
	Ref<Resource> graphic_Mesh_or_PackedScene;
	Ref<Resource> get_graphic_Mesh_or_PackedScene();
	void set_graphic_Mesh_or_PackedScene(Ref<Resource> v);

	Ref<Mesh> collision_mesh;
	Ref<Mesh> get_collision_mesh();
	void set_collision_mesh(Ref<Mesh> v);

public:
	MeshInstance3D *col = nullptr;
	Node3D *graph = nullptr;
};
} // namespace editor
