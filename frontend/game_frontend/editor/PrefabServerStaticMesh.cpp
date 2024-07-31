#include "PrefabServerStaticMesh.hpp"

namespace editor
{
PrefabServerStaticMesh::PrefabServerStaticMesh()
{
}
PrefabServerStaticMesh::~PrefabServerStaticMesh() {}

void PrefabServerStaticMesh::_bind_methods() {
	REGISTER_PROPERTY(PrefabServerStaticMesh, isTrullyStaticForMerge,
					  Variant::Type::BOOL, "isTrullyStaticForMerge");
}
} // namespace editor
