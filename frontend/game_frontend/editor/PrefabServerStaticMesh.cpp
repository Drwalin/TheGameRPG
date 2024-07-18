#include "../../../common/include/EntityComponents.hpp"
#include "../../../common/include/RegistryComponent.hpp"

#include "PrefabServerStaticMesh.hpp"

namespace editor
{
PrefabServerStaticMesh::PrefabServerStaticMesh() {}
PrefabServerStaticMesh::~PrefabServerStaticMesh() {}

void PrefabServerStaticMesh::Serialize(icon7::ByteWriter &writer)
{
	std::string graphicPath;
	if (graphic_Mesh_or_PackedScene.is_null() == false &&
		graphic_Mesh_or_PackedScene.is_valid()) {
		graphicPath = graphic_Mesh_or_PackedScene->get_path().utf8().ptr();
		if (graphicPath.find(RES_PREFIX) == 0) {
			graphicPath = graphicPath.replace(0, RES_PREFIX.size(), "");
		}
	}

	std::string collisionPath;
	if (collision_mesh.is_null() == false && collision_mesh.is_valid()) {
		collisionPath = collision_mesh->get_path().utf8().ptr();
		if (collisionPath.find(RES_PREFIX) == 0) {
			collisionPath = collisionPath.replace(0, RES_PREFIX.size(), "");
		}
	}

	ComponentStaticTransform transform = ToGame(get_global_transform());
	ComponentModelName model;
	model.modelName = graphicPath;
	ComponentStaticCollisionShapeName collision;
	collision.shapeName = collisionPath;

	reg::Registry::Serialize(transform, writer);
	reg::Registry::Serialize(model, writer);
	reg::Registry::Serialize(collision, writer);
}

void PrefabServerStaticMesh::_bind_methods()
{
	REGISTER_PROPERTY_RESOURCE(PrefabServerStaticMesh,
							   graphic_Mesh_or_PackedScene,
							   Variant::Type::OBJECT, "Resource", "scene");
	REGISTER_PROPERTY_RESOURCE(PrefabServerStaticMesh, collision_mesh,
							   Variant::Type::OBJECT, "Mesh", "mesh");
}

void PrefabServerStaticMesh::_ready()
{
	PrefabServerBase::_ready();
	col = nullptr;
	graph = nullptr;
}

void PrefabServerStaticMesh::_process(double dt)
{
	PrefabServerBase::_process(dt);
	if ((col != nullptr) != GameEditorConfig::render_collision) {
		RecreateCollision();
	}

	if ((graph != nullptr) != GameEditorConfig::render_graphic) {
		RecreateGraphic();
	}
}

void PrefabServerStaticMesh::RecreateCollision()
{
	RecreateResourceRenderer(&col, &collision_mesh,
							 GameEditorConfig::render_collision);
}

void PrefabServerStaticMesh::RecreateGraphic()
{
	RecreateResourceRenderer(&graph, &graphic_Mesh_or_PackedScene,
							 GameEditorConfig::render_graphic);
}

Ref<Resource> PrefabServerStaticMesh::get_graphic_Mesh_or_PackedScene()
{
	return graphic_Mesh_or_PackedScene;
}
void PrefabServerStaticMesh::set_graphic_Mesh_or_PackedScene(Ref<Resource> v)
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

Ref<Mesh> PrefabServerStaticMesh::get_collision_mesh()
{
	return collision_mesh;
}
void PrefabServerStaticMesh::set_collision_mesh(Ref<Mesh> v)
{
	collision_mesh = v;
	RecreateCollision();
}

} // namespace editor
