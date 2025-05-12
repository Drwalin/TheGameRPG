#pragma once

#include <godot_cpp/classes/packed_scene.hpp>
#include <godot_cpp/classes/mesh.hpp>
#include <godot_cpp/classes/csg_primitive3d.hpp>

#include <godot_cpp/variant/utility_functions.hpp>
#include <godot_cpp/godot.hpp>
#include <godot_cpp/core/class_db.hpp>

#include <gdextension_interface.h>
#include <godot_cpp/core/defs.hpp>
#include <godot_cpp/godot.hpp>
#include <godot_cpp/core/engine_ptrcall.hpp>
#include <godot_cpp/classes/mesh_instance3d.hpp>

#include <godot_cpp/classes/node3d.hpp>

#include <icon7/Forward.hpp>

#include "../../../common/include/EntityComponents.hpp"

#include "EditorConfig.hpp"

using namespace godot;

namespace editor
{
class EntityBase : public Node3D
{
	GDCLASS(EntityBase, Node3D)
public: // Godot bound functions
	EntityBase();
	virtual ~EntityBase();
	static void _bind_methods();

	void _ready() override;
	void _process(double dt) override;

	virtual void Serialize(icon7::ByteWriter &writer);

	static String GetRandomString();

public:
	Ref<Resource> graphic_Mesh_or_PackedScene;
	Ref<Resource> get_graphic_Mesh_or_PackedScene();
	void set_graphic_Mesh_or_PackedScene(Ref<Resource> v);

	String graphicInstanceName = "";
	String get_graphicInstanceName();
	void set_graphicInstanceName(String v);
	
private:
	bool IsTrigger();
	bool IsMovingCharacter();

private:
	void SerializeGraphics(icon7::ByteWriter &writer);
	void SerializeCollisions(icon7::ByteWriter &writer);
	void SerializeComponents(icon7::ByteWriter &writer);

	void RecreateResourceRenderer(Ref<Resource> resource);
	void GenerateTriCollisionForAll(Node *node);

	__InnerShape GetShape(CSGPrimitive3D *primitive, Transform3D invTransform);
};

extern godot::Transform3D ToGodot(ComponentStaticTransform t);
extern ComponentStaticTransform ToGame(godot::Transform3D t);

} // namespace editor
