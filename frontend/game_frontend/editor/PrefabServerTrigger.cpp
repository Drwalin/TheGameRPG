#include "../../../server/include/EntityGameComponents.hpp"
#include "../../../common/include/RegistryComponent.hpp"

#include "../GodotGlm.hpp"

#include "EditorConfig.hpp"

#include "PrefabServerTrigger.hpp"

namespace editor
{
PrefabServerTrigger::PrefabServerTrigger() {}
PrefabServerTrigger::~PrefabServerTrigger() {}

void PrefabServerTrigger::Serialize(uint16_t higherLevelComponentsCount,
									icon7::ByteWriter &writer)
{
	PrefabServerBase::Serialize(higherLevelComponentsCount + 3, writer);

	std::string onEnter = onTriggerEnter.utf8().ptr();
	named_callbacks::registry_entries::OnTriggerEnterExit onTriggerEnter{
		onEnter, onEnter, {}, nullptr, nullptr};
	std::string onExit = onTriggerExit.utf8().ptr();
	named_callbacks::registry_entries::OnTriggerEnterExit onTriggerExit{
		onExit, onExit, {}, nullptr, nullptr};
	ComponentTrigger onTrigger;
	onTrigger.onEnter = &onTriggerEnter;
	onTrigger.onExit = &onTriggerExit;
	reg::Registry::Serialize(onTrigger, writer);

	std::string realmName = teleportRealm.utf8().ptr();
	ComponentTeleport teleport;
	teleport.realmName = realmName;
	teleport.position = ToGlm(teleportPosition);
	reg::Registry::Serialize(teleport, writer);

	ComponentStaticTransform transform = ToGame(get_global_transform());
	reg::Registry::Serialize(transform, writer);
}

void PrefabServerTrigger::_bind_methods()
{
	REGISTER_PROPERTY(PrefabServerTrigger, onTriggerEnter,
					  Variant::Type::STRING, "OnEnter");
	REGISTER_PROPERTY(PrefabServerTrigger, onTriggerExit, Variant::Type::STRING,
					  "OnExit");
	REGISTER_PROPERTY(PrefabServerTrigger, teleportRealm, Variant::Type::STRING,
					  "TeleportRealmName");
	REGISTER_PROPERTY(PrefabServerTrigger, teleportPosition,
					  Variant::Type::VECTOR3, "TeleportPosition");
}

void PrefabServerTrigger::_ready()
{
	PrefabServerBase::_ready();
	renderBox = new CSGBox3D();
	renderBox->set_use_collision(true);
	add_child(renderBox);
	renderBox->set_owner(this);
}

void PrefabServerTrigger::_process(double dt)
{
	PrefabServerBase::_process(dt);
	renderBox->set_visible(GameEditorConfig::render_triggers);
}

String PrefabServerTrigger::get_onTriggerEnter() { return onTriggerEnter; }
void PrefabServerTrigger::set_onTriggerEnter(String v) { onTriggerEnter = v; }

String PrefabServerTrigger::get_onTriggerExit() { return onTriggerExit; }
void PrefabServerTrigger::set_onTriggerExit(String v) { onTriggerExit = v; }

String PrefabServerTrigger::get_teleportRealm() { return teleportRealm; }
void PrefabServerTrigger::set_teleportRealm(String v) { teleportRealm = v; }

Vector3 PrefabServerTrigger::get_teleportPosition() { return teleportPosition; }
void PrefabServerTrigger::set_teleportPosition(Vector3 v)
{
	teleportPosition = v;
}

} // namespace editor
