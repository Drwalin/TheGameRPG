#include "../GodotGlm.hpp"

#include "../../../common/include/EntityComponents.hpp"
#include "../../../common/include/RegistryComponent.hpp"

#include "PrefabServerStaticMesh.hpp"

namespace editor
{
PrefabServerStaticMesh::PrefabServerStaticMesh() {}
PrefabServerStaticMesh::~PrefabServerStaticMesh() {}

void PrefabServerStaticMesh::Serialize(uint16_t higherLevelComponentsCount,
									   icon7::ByteWriter &writer)
{
	PrefabServerBase::Serialize(higherLevelComponentsCount + 3, writer);

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
	
	ComponentStaticTransform transform{
		ToGlm(get_global_position()),
		ToGlm(get_global_basis().get_quaternion()),
		ToGlm(get_scale())};
	ComponentModelName model{graphicPath};
	ComponentStaticCollisionShapeName collision{collisionPath};

	reg::Registry::Serialize(transform, writer);
	reg::Registry::Serialize(model, writer);
	reg::Registry::Serialize(collision, writer);
}
} // namespace editor
