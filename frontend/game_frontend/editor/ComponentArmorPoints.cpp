#include "../../../common/include/RegistryComponent.hpp"
#include "../../../common/include/ComponentCharacterSheet.hpp"

#include "../GameClientFrontend.hpp"

#include "EditorConfig.hpp"
#include "ComponentArmorPoints.hpp"

namespace editor
{
ComponentArmorPoints::ComponentArmorPoints() {}
ComponentArmorPoints::~ComponentArmorPoints() {}

void ComponentArmorPoints::Serialize(icon7::ByteWriter &writer)
{
	ComponentCharacterSheet_Protection prot;
	prot.armorPoints = armorPoints;
	reg::Registry::SerializePersistent(GameClientFrontend::singleton->realm,
									   prot, writer);
}

void ComponentArmorPoints::_bind_methods()
{
	REGISTER_PROPERTY(ComponentArmorPoints, armorPoints, Variant::Type::INT,
					  "armorPoints");
}
} // namespace editor
