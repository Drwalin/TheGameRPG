#pragma once
#if false

#include "PrefabServerStaticMesh.hpp"

namespace editor
{
class PrefabServerOpenableSingleDoor : public PrefabServerStaticMesh
{
	GDCLASS(PrefabServerOpenableSingleDoor, Node3D)
public: // Godot bound functions
	PrefabServerOpenableSingleDoor();
	virtual ~PrefabServerOpenableSingleDoor();
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
	bool openedState;
	inline bool get_openedState()
	{
		return openedState;
	}
	inline void set_openedState(bool v)
	{
		auto pos = get_global_position();
		auto basis = get_global_basis();
		auto scale = get_scale();
		
		
		
		
		
		
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
#endif
