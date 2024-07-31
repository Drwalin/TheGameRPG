#include "../../../common/include/EntityComponents.hpp"
#include "../../../common/include/RegistryComponent.hpp"

#include "../GameClientFrontend.hpp"

#include "PrefabServerStaticMesh_Base.hpp"

namespace editor
{
PrefabServerStaticMesh_Base::PrefabServerStaticMesh_Base() {}
PrefabServerStaticMesh_Base::~PrefabServerStaticMesh_Base() {}

void PrefabServerStaticMesh_Base::Serialize(icon7::ByteWriter &writer)
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

	reg::Registry::SerializePersistent(GameClientFrontend::singleton->realm,
									   transform, writer);
	reg::Registry::SerializePersistent(GameClientFrontend::singleton->realm,
									   model, writer);
	reg::Registry::SerializePersistent(GameClientFrontend::singleton->realm,
									   collision, writer);
}

Transform3D PrefabServerStaticMesh_Base::GetMergingData(
	Ref<Resource> *graphicMeshOrPackedScene, Ref<Mesh> *collisionMesh)
{
	*graphicMeshOrPackedScene = graphic_Mesh_or_PackedScene;
	*collisionMesh = collision_mesh;
	return get_global_transform();
}

void PrefabServerStaticMesh_Base::_bind_methods()
{
	REGISTER_PROPERTY_RESOURCE(PrefabServerStaticMesh_Base,
							   graphic_Mesh_or_PackedScene,
							   Variant::Type::OBJECT, "Resource", "scene");
	REGISTER_PROPERTY_RESOURCE(PrefabServerStaticMesh_Base, collision_mesh,
							   Variant::Type::OBJECT, "Mesh", "mesh");
}

void PrefabServerStaticMesh_Base::_ready()
{
	PrefabServerBase::_ready();
	col = nullptr;
	graph = nullptr;
}

void PrefabServerStaticMesh_Base::_process(double dt)
{
	PrefabServerBase::_process(dt);
	if ((col != nullptr) != GameEditorConfig::render_collision) {
		RecreateCollision();
	}

	if ((graph != nullptr) != GameEditorConfig::render_graphic) {
		RecreateGraphic();
	}
}

void PrefabServerStaticMesh_Base::RecreateCollision()
{
	RecreateResourceRenderer(&col, &collision_mesh,
							 GameEditorConfig::render_collision);
}

void PrefabServerStaticMesh_Base::RecreateGraphic()
{
	RecreateResourceRenderer(&graph, &graphic_Mesh_or_PackedScene,
							 GameEditorConfig::render_graphic);
}

Ref<Resource> PrefabServerStaticMesh_Base::get_graphic_Mesh_or_PackedScene()
{
	return graphic_Mesh_or_PackedScene;
}
void PrefabServerStaticMesh_Base::set_graphic_Mesh_or_PackedScene(
	Ref<Resource> v)
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

Ref<Mesh> PrefabServerStaticMesh_Base::get_collision_mesh()
{
	return collision_mesh;
}
void PrefabServerStaticMesh_Base::set_collision_mesh(Ref<Mesh> v)
{
	collision_mesh = v;
	RecreateCollision();
}

} // namespace editor
