#include <random>
#include <functional>

#include <godot_cpp/variant/transform3d.hpp>
#include <godot_cpp/variant/basis.hpp>
#include "godot_cpp/variant/utility_functions.hpp"
#include <godot_cpp/classes/csg_primitive3d.hpp>
#include <godot_cpp/classes/csg_sphere3d.hpp>
#include <godot_cpp/classes/csg_box3d.hpp>
#include <godot_cpp/classes/csg_cylinder3d.hpp>

#include "../../../ICon7/include/icon7/ByteWriter.hpp"

#include "../../thirdparty/Collision3D/include/collision3d/CollisionShapes_AnyOrCompound.hpp"
#include "../../thirdparty/Collision3D/include/collision3d/CollisionShapes_Primitives.hpp"

#include "../../../common/include/EntityComponents.hpp"
#include "../../../common/include/RegistryComponent.hpp"

#include "../GodotGlm.hpp"
#include "../GameClientFrontend.hpp"

#include "EditorConfig.hpp"
#include "ComponentBase.hpp"
#include "ComponentTrigger.hpp"
#include "ComponentCharacterSheet.hpp"
#include "EntityBase.hpp"

namespace editor
{
using namespace Collision3D;

EntityBase::EntityBase() {}
EntityBase::~EntityBase() {}

void EntityBase::_bind_methods()
{
	REGISTER_PROPERTY_RESOURCE(EntityBase, graphic_Mesh_or_PackedScene,
							   Variant::Type::OBJECT, "Resource", "scene");
}

void EntityBase::_ready() { CheckRemoveUnneededChildren(); }

void EntityBase::_process(double dt)
{
	lastCheck += dt;
	if (lastCheck > 1.0f) {
		CheckRemoveUnneededChildren();
	}
	CheckRotationAndScale();
}

void EntityBase::CheckRemoveUnneededChildren()
{
	lastCheck = -1.f;
	for (int i = 0; i < get_child_count(true); ++i) {
		Node *n = get_child(i, true);
		if (Object::cast_to<CSGPrimitive3D>(n)) {
			continue;
		}
		if (Object::cast_to<ComponentBase>(n)) {
			continue;
		}
		if (n->get_name() == graphicInstanceName) {
			continue;
		}
		remove_child(n);
		--i;
	}
}

void EntityBase::CheckRotationAndScale()
{
	Transform3D trans = get_global_transform();
	
	if (trans == lastGlobalTransform) {
		return;
	}
			
	lastGlobalTransform = trans;
	
	Basis basis = trans.get_basis();
	glm::vec3 s = ToGlm(basis.get_scale());
	
	Vector3 axis;
	float angle;
	basis.get_rotation_axis_angle(axis, angle);
	glm::vec3 a = glm::abs(ToGlm(axis));
	
	bool mod = false;
	
	if (a.x > 0.0001 || a.z > 0.0001) {
		Vector3 euler = basis.get_euler();
		euler.x = euler.z = 0;
		basis = Basis::from_euler(euler);
		mod = true;
	}
	
	if (s.x != s.y || s.x != s.z || mod) {
		float sf = glm::maxcomp(glm::abs(s));
		if (!mod) {
			mod = true;
			basis = basis.get_rotation_quaternion();
		}
		basis = basis.scaled(Vector3(sf, sf, sf));
		trans.set_basis(basis);
		this->set_global_transform(trans);
	}
}

void EntityBase::Serialize(icon7::ByteWriter &writer)
{
	if (!IsMovingCharacter()) {
		reg::Registry::SerializePersistent(
			GameClientFrontend::singleton->realm,
			ComponentStaticTransform{ToGame(get_global_transform())}, writer);
	}

	SerializeComponents(writer);
	SerializeCollisions(writer);
	SerializeGraphics(writer);
	writer.op("");
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
	func(this);

	ComponentCollisionShape shape;
	Collision3D::AnyShape &is = shape.shape;

	Transform3D inv = get_global_transform().affine_inverse();

	if (primitives.size() == 1) {
		CSGPrimitive3D *csg = primitives[0];
		is = GetShape(csg, inv);
	} else if (primitives.size() > 1) {
		Collision3D::CompoundPrimitive cs;
		for (CSGPrimitive3D *csg : primitives) {
			cs.primitives.push_back(GetShape(csg, inv));
		}
		is = Collision3D::AnyShape(std::move(cs), {});
	} else {
		return;
	}

	reg::Registry::SerializePersistent(GameClientFrontend::singleton->realm,
									   shape, writer);
}

Collision3D::AnyPrimitive EntityBase::GetShape(CSGPrimitive3D *primitive, Transform3D inv)
{
	Transform3D trans =
		primitive->get_transform(); // inv * primitive->get_global_transform();

	Vector3 scale = primitive->get_global_transform().get_basis().get_scale();
	
	Collision3D::AnyPrimitive shape;
	if (auto *cyl = Object::cast_to<CSGCylinder3D>(primitive)) {
		UtilityFunctions::print("Creating from editor: cyl");
		Collision3D::Cylinder s;
		s.radius = cyl->get_radius();
		s.height = cyl->get_height();
		s.radius *= scale.x;
		s.height *= scale.y;
		trans = trans.translated(Vector3(0, -s.height / 2, 0));
		shape = {std::move(s), {}};
	} else if (auto *box = Object::cast_to<CSGBox3D>(primitive)) {
		UtilityFunctions::print("Creating from editor: vertbox");
		Collision3D::VertBox s;
		s.halfExtents = ToGlm(box->get_size()) * 0.5f;
		s.halfExtents *= ToGlm(scale);
		trans = trans.translated(Vector3(0, -s.halfExtents.y, 0));
		shape = {std::move(s), {}};
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

	shape.trans = ToGame(trans).trans;
	return shape;
}

void EntityBase::SerializeComponents(icon7::ByteWriter &writer)
{
	for (int i = 0; i < get_child_count(true); ++i) {
		if (auto *comp = Object::cast_to<ComponentBase>(get_child(i, true))) {
			comp->Serialize(writer);
		}
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

Ref<Resource> EntityBase::get_graphic_Mesh_or_PackedScene()
{
	return graphic_Mesh_or_PackedScene;
}
void EntityBase::set_graphic_Mesh_or_PackedScene(Ref<Resource> v)
{
	graphic_Mesh_or_PackedScene = Ref<Resource>{};

	Ref<PackedScene> packedScene = v;
	if (packedScene.is_null() == false && packedScene.is_valid()) {
		graphic_Mesh_or_PackedScene = packedScene;
		RecreateResourceRenderer(packedScene);
	}

	Ref<Mesh> mesh = v;
	if (mesh.is_null() == false && mesh.is_valid()) {
		graphic_Mesh_or_PackedScene = mesh;
		RecreateResourceRenderer(mesh);
	}
}

godot::Transform3D ToGodot(ComponentStaticTransform t)
{
	return ::ToGodot(t.trans).scaled({t.scale, t.scale, t.scale});
	;
}

ComponentStaticTransform ToGame(godot::Transform3D t)
{
	return {{ToGlm(t.get_origin()),
			 Collision3D::Rotation::FromRadians(
				 t.get_basis().get_quaternion().get_euler().y)},
			1.0f};
}

bool EntityBase::IsTrigger()
{
	for (int i = 0; i < get_child_count(true); ++i) {
		if (Object::cast_to<editor::ComponentTrigger>(get_child(i, true))) {
			return true;
		}
	}
	return false;
}

bool EntityBase::IsMovingCharacter()
{
	for (int i = 0; i < get_child_count(true); ++i) {
		if (Object::cast_to<editor::ComponentCharacterSheet>(
				get_child(i, true))) {
			return true;
		}
	}
	return false;
}

} // namespace editor
