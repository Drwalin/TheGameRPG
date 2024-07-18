#pragma once

#include <godot_cpp/classes/csg_box3d.hpp>

#include "PrefabServerBase.hpp"

namespace editor
{
class PrefabServerNPC : public PrefabServerBase
{
	GDCLASS(PrefabServerNPC, PrefabServerBase)
public: // Godot bound functions
	PrefabServerNPC();
	virtual ~PrefabServerNPC();
	static void _bind_methods();

	void _ready() override;
	void _process(double dt) override;

	virtual void Serialize(icon7::ByteWriter &writer) override;

	void RecreateGraphic();

public: // variables
	Ref<Resource> graphic_Mesh_or_PackedScene;
	Ref<Resource> get_graphic_Mesh_or_PackedScene();
	void set_graphic_Mesh_or_PackedScene(Ref<Resource> v);

	String nameNPC = "";
	String get_nameNPC() { return nameNPC; }
	void set_nameNPC(String v) { nameNPC = v; }

	String aiTickName = "";
	String get_aiTickName() { return aiTickName; }
	void set_aiTickName(String v) { aiTickName = v; }

	String OnUse = "";
	String get_OnUse() { return OnUse; }
	void set_OnUse(String v) { OnUse = v; }

	double height = 1.85;
	double get_height() { return height; }
	void set_height(double v) { height = v; }

	double width = 0.5;
	double get_width() { return width; }
	void set_width(double v) { width = v; }

	double movementSpeed = 5.0;
	double get_movementSpeed() { return movementSpeed; }
	void set_movementSpeed(double v) { movementSpeed = v; }

	double stepHeight = 0.25;
	double get_stepHeight() { return stepHeight; }
	void set_stepHeight(double v) { stepHeight = v; }

private:
	Node3D *graph = nullptr;
};
} // namespace editor
