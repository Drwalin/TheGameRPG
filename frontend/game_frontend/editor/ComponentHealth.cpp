#include "../../../common/include/RegistryComponent.hpp"
#include "../../../common/include/ComponentCharacterSheet.hpp"

#include "../GameClientFrontend.hpp"

#include "EditorConfig.hpp"
#include "ComponentHealth.hpp"

namespace editor
{
ComponentHealth::ComponentHealth() {}
ComponentHealth::~ComponentHealth() {}

void ComponentHealth::Serialize(icon7::ByteWriter &writer)
{
	UtilityFunctions::print("Saving health");
	reg::Registry::SerializePersistent(
		GameClientFrontend::singleton->realm,
		ComponentCharacterSheet_Health(initialMaxHp, initialHp), writer);
}

void ComponentHealth::_bind_methods()
{
	REGISTER_PROPERTY(ComponentHealth, initialMaxHp, Variant::Type::INT,
					  "initialMaxHp");
	REGISTER_PROPERTY(ComponentHealth, initialHp, Variant::Type::INT,
					  "initialHp");
}
} // namespace editor
