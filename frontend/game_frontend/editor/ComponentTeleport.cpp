#include "../../../server/include/EntityGameComponents.hpp"
#include "../../../common/include/RegistryComponent.hpp"

#include "../GodotGlm.hpp"
#include "../GameClientFrontend.hpp"

#include "EditorConfig.hpp"
#include "ComponentTeleport.hpp"

namespace editor
{
ComponentTeleport::ComponentTeleport() {}
ComponentTeleport::~ComponentTeleport() {}

void ComponentTeleport::Serialize(icon7::ByteWriter &writer)
{
	UtilityFunctions::print("Saving teleport");
	std::string realmName = teleportRealm.utf8().ptr();
	::ComponentTeleport teleport;
	teleport.realmName = realmName;
	teleport.position = ToGlm(teleportPosition);
	reg::Registry::SerializePersistent(GameClientFrontend::singleton->realm,
									   teleport, writer);
}

void ComponentTeleport::_bind_methods()
{
	REGISTER_PROPERTY(ComponentTeleport, teleportRealm, Variant::Type::STRING,
					  "TeleportRealmName");
	REGISTER_PROPERTY(ComponentTeleport, teleportPosition,
					  Variant::Type::VECTOR3, "TeleportPosition");
}

String ComponentTeleport::get_teleportRealm() { return teleportRealm; }
void ComponentTeleport::set_teleportRealm(String v) { teleportRealm = v; }

Vector3 ComponentTeleport::get_teleportPosition() { return teleportPosition; }
void ComponentTeleport::set_teleportPosition(Vector3 v) { teleportPosition = v; }

} // namespace editor
