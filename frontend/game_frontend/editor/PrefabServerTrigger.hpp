#pragma once

#include <godot_cpp/classes/csg_box3d.hpp>

#include "PrefabServerBase.hpp"

namespace editor
{
class PrefabServerTrigger : public PrefabServerBase
{
	GDCLASS(PrefabServerTrigger, PrefabServerBase)
public: // Godot bound functions
	PrefabServerTrigger();
	virtual ~PrefabServerTrigger();
	static void _bind_methods();

	void _ready() override;
	void _process(double dt) override;

	virtual void Serialize(uint16_t higherLevelComponentsCount,
						   icon7::ByteWriter &writer) override;

public: // variables
	String onTriggerEnter = "";
	String get_onTriggerEnter();
	void set_onTriggerEnter(String v);

	String onTriggerExit = "";
	String get_onTriggerExit();
	void set_onTriggerExit(String v);

	String teleportRealm = "";
	String get_teleportRealm();
	void set_teleportRealm(String v);

	Vector3 teleportPosition = {0, 0, 0};
	Vector3 get_teleportPosition();
	void set_teleportPosition(Vector3 v);

private:
	class CSGBox3D *renderBox = nullptr;
};
} // namespace editor
