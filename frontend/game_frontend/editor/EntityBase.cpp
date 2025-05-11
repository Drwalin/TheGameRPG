#include <random>
#include <functional>

#include <godot_cpp/variant/transform3d.hpp>
#include <godot_cpp/variant/basis.hpp>
#include <godot_cpp/classes/csg_primitive3d.hpp>
#include <godot_cpp/classes/csg_sphere3d.hpp>
#include <godot_cpp/classes/csg_box3d.hpp>
#include <godot_cpp/classes/csg_cylinder3d.hpp>

#include "../../../thirdparty/Collision3D/include/collision3d/CollisionShapes.hpp"

#include "../../../common/include/EntityComponents.hpp"
#include "../../../common/include/RegistryComponent.hpp"

#include "../GodotGlm.hpp"
#include "../GameClientFrontend.hpp"

#include "EditorConfig.hpp"
#include "EntityBase.hpp"

namespace editor
{
EntityBase::EntityBase() {}
EntityBase::~EntityBase() {}

void EntityBase::_bind_methods() {}

void EntityBase::_ready() {}

void EntityBase::_process(double dt) {}

void EntityBase::Serialize(icon7::ByteWriter &writer)
{
	SerializeGraphics(writer);
	SerializeCollisions(writer);
	SerializeComponents(writer);
}

void EntityBase::SerializeGraphics(icon7::ByteWriter &writer)
{
	if (graphic_Mesh_or_PackedScene.is_null()) {
		return;
	}
	std::string graphicPath;
	if (graphic_Mesh_or_PackedScene.is_null() == false &&
		graphic_Mesh_or_PackedScene.is_valid()) {
		graphicPath = graphic_Mesh_or_PackedScene->get_path().utf8().ptr();
		if (graphicPath.find(RES_PREFIX) == 0) {
			graphicPath = graphicPath.replace(0, RES_PREFIX.size(), "");
		}
	}

	reg::Registry::SerializePersistent(
		GameClientFrontend::singleton->realm,
		ComponentStaticTransform{ToGame(get_global_transform())}, writer);

	reg::Registry::SerializePersistent(GameClientFrontend::singleton->realm,
									   ComponentModelName{graphicPath}, writer);
}

void EntityBase::SerializeCollisions(icon7::ByteWriter &writer)
{
	std::vector<CSGPrimitive3D *> primitives;
	std::function<void(Node *)> func;
	func = [&](Node *n) {
		for (int i = 0; i < n->get_child_count(false); ++i) {
			Node *child = n->get_child(i, false);
			if (CSGPrimitive3D *csg = Object::cast_to<CSGPrimitive3D>(child)) {
				primitives.push_back(csg);
				func(child);
			}
		}
	};

	ComponentCollisionShape shape;
	__InnerShape &is = shape.shape;

	Transform3D inv = get_global_transform().affine_inverse();

	if (primitives.size() == 1) {
		CSGPrimitive3D *csg = primitives[0];
		is = GetShape(csg, inv);
	} else if (primitives.size() > 1) {
		CompoundShape cs;
		for (CSGPrimitive3D *csg : primitives) {
			cs.shapes->push_back(GetShape(csg, inv));
		}
		is.type = __InnerShape::COMPOUND_SHAPE;
		is.shape = cs;
		is.trans = {};
	}

	reg::Registry::SerializePersistent(GameClientFrontend::singleton->realm,
									   shape, writer);
}

__InnerShape EntityBase::GetShape(CSGPrimitive3D *primitive, Transform3D inv)
{
	Transform3D trans = inv * primitive->get_global_transform();

	__InnerShape shape;
	if (auto *cyl = Object::cast_to<CSGCylinder3D>(primitive)) {
		Collision3D::Cylinder s;
		s.radius = cyl->get_radius();
		s.height = cyl->get_height();
		trans = trans.translated(Vector3(0, -s.height / 2, 0));
		shape.type = __InnerShape::CYLINDER;
		shape.shape = s;
	} else if (auto *box = Object::cast_to<CSGBox3D>(primitive)) {
		Collision3D::VertBox s;
		s.halfExtents = ToGlm(box->get_size()) * 0.5f;
		trans = trans.translated(Vector3(0, -s.halfExtents.y, 0));
		shape.type = __InnerShape::VERTBOX;
		shape.shape = s;
	} else if (/*auto *sphere =*/Object::cast_to<CSGSphere3D>(primitive)) {
		UtilityFunctions::print(
			"Sphere colliion shape is not implemented yet yet");
		/*
		Collision3D::Sphere s;
		s.radius = sphere->get_radius();
		shape.type = __InnerShape::SPHERE;
		shape.shape = s;
		*/
	} else {
		UtilityFunctions::print(
			"Unknown collsion primitive shape for serialization: ",
			primitive->to_string());
	}

	shape.trans = {ToGame(trans)};
	return shape;
}

void EntityBase::SerializeComponents(icon7::ByteWriter &writer)
{
	for (int i = 0; i < get_child_count(true); ++i) {
	}
}

String EntityBase::GetRandomString()
{
	static std::random_device rd;
	static std::mt19937_64 mt64(rd());

	std::string str;
	str.resize(64);
	sprintf(str.data(), "%lX%lX", (uint64_t)mt64(), (uint64_t)mt64());
	return String::utf8(str.c_str());
}

void EntityBase::RecreateResourceRenderer(Ref<Resource> resource)
{
	if (graphicInstanceName != "") {
		for (int i = 0; i < get_child_count(true); ++i) {
			Node *child = get_child(i, true);
			if (child->get_name() == graphicInstanceName) {
				child->queue_free();
			}
		}
		graphicInstanceName = "";
	}

	if (!GameEditorConfig::render_graphic) {
		return;
	}

	if (resource.is_valid() && !resource.is_null()) {
		Node *node = nullptr;

		Ref<PackedScene> packedScene = resource;
		Ref<Mesh> mesh = resource;
		if (packedScene.is_null() == false && packedScene.is_valid()) {
			node = Object::cast_to<Node3D>(packedScene->instantiate());
		} else if (mesh.is_null() == false && mesh.is_valid()) {
			MeshInstance3D *m = memnew(MeshInstance3D);
			m->set_mesh(mesh);
			node = m;
		}

		if (node) {
			add_child(node, true, Node::InternalMode::INTERNAL_MODE_BACK);
			node->set_owner(this);
			node->set_name(GetRandomString());
			graphicInstanceName = node->get_name();
			GenerateTriCollisionForAll(node);
		}
	}
}

void EntityBase::GenerateTriCollisionForAll(Node *node)
{
	if (MeshInstance3D *mesh = Object::cast_to<MeshInstance3D>(node)) {
		mesh->create_trimesh_collision();
	}
	for (int i = 0; i < node->get_child_count(true); ++i) {
		GenerateTriCollisionForAll(
			(Node *)(Object *)(node->get_child(i, true)));
	}
}

godot::Transform3D ToGodot(ComponentStaticTransform t)
{
	return Transform3D(Basis(::ToGodot(t.rot), ::ToGodot(t.scale)),
					   ::ToGodot(t.pos));
}

ComponentStaticTransform ToGame(godot::Transform3D t)
{
	return {ToGlm(t.get_origin()),
			ToGlm(t.get_basis().get_rotation_quaternion()),
			ToGlm(t.get_basis().get_scale())};
}

} // namespace editor
