#include <random>

#include <godot_cpp/variant/transform3d.hpp>
#include <godot_cpp/variant/basis.hpp>

#include "../GodotGlm.hpp"

#include "PrefabServerBase.hpp"

namespace editor
{
PrefabServerBase::PrefabServerBase() {}
PrefabServerBase::~PrefabServerBase() {}

void PrefabServerBase::_bind_methods()
{
	REGISTER_PROPERTY(PrefabServerBase, isTrullyStaticForMerge,
					  Variant::Type::BOOL, "isTrullyStaticForMerge");
}

void PrefabServerBase::ClearChildren()
{
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
}

void PrefabServerBase::_ready() { ClearChildren(); }

void PrefabServerBase::_process(double dt) {}

void PrefabServerBase::Serialize(icon7::ByteWriter &writer) {}

String PrefabServerBase::GetRandomString()
{
	static std::random_device rd;
	static std::mt19937_64 mt64(rd());

	std::string str;
	str.resize(64);
	sprintf(str.data(), "Name%lX%lX", (uint64_t)mt64(), (uint64_t)mt64());
	return String::utf8(str.c_str());
}

template <typename TNode, typename TRes>
void PrefabServerBase::_RecreateResourceRenderer(TNode **nodeStorage,
												 Ref<TRes> *resourceStorage,
												 bool enable)
{
	if (*nodeStorage) {
		remove_child(*nodeStorage);
		(*nodeStorage)->queue_free();
		(*nodeStorage) = nullptr;
	}
	if (enable) {
		if (resourceStorage->is_valid() && !resourceStorage->is_null()) {

			if constexpr (std::is_same_v<TNode, Node3D>) {
				Ref<PackedScene> packedScene = *resourceStorage;
				if (packedScene.is_null() == false && packedScene.is_valid()) {
					(*nodeStorage) =
						Object::cast_to<Node3D>(packedScene->instantiate());
					(*nodeStorage)->set_name(GetRandomString());
					add_child(*nodeStorage);
					(*nodeStorage)->set_owner(this);
				}
			}

			Ref<Mesh> mesh = *resourceStorage;
			if (mesh.is_null() == false && mesh.is_valid()) {
				MeshInstance3D *m = new MeshInstance3D();
				m->set_mesh(mesh);
				(*nodeStorage) = m;
				(*nodeStorage)->set_name(GetRandomString());
				add_child(*nodeStorage);
				(*nodeStorage)->set_owner(this);
			}
		}
	}
	if (*nodeStorage) {
		GenerateTriCollisionForAll(*nodeStorage);
	}
}

void PrefabServerBase::RecreateResourceRenderer(
	godot::MeshInstance3D **nodeStorage, Ref<godot::Mesh> *resourceStorage,
	bool enable)
{
	_RecreateResourceRenderer(nodeStorage, resourceStorage, enable);
}

void PrefabServerBase::RecreateResourceRenderer(
	godot::Node3D **nodeStorage, Ref<godot::Resource> *resourceStorage,
	bool enable)
{
	_RecreateResourceRenderer(nodeStorage, resourceStorage, enable);
}

void PrefabServerBase::GenerateTriCollisionForAll(Node3D *node)
{
	if (MeshInstance3D *mesh = Object::cast_to<MeshInstance3D>(node)) {
		mesh->create_trimesh_collision();
	}
	TypedArray<Node3D> children = node->get_children(true);
	for (int i = 0; i < children.size(); ++i) {
		GenerateTriCollisionForAll((Node3D *)(Object *)(children[i]));
	}
}

godot::Transform3D ToGodot(ComponentStaticTransform t)
{
	return Transform3D(Basis(::ToGodot(t.rot), ::ToGodot(t.scale)),
					   ::ToGodot(t.pos));
}

ComponentStaticTransform ToGame(godot::Transform3D t)
{
	ComponentStaticTransform ret;
	ret.pos = ToGlm(t.get_origin());
	ret.rot = ToGlm(t.get_basis().get_rotation_quaternion());
	ret.scale = ToGlm(t.get_basis().get_scale());
	return ret;
}

} // namespace editor
