#include "../../../common/include/RegistryComponent.hpp"
#include "../../../common/include/ComponentCharacterSheet.hpp"

#include "../GameClientFrontend.hpp"

#include "EditorConfig.hpp"
#include "ComponentHealthRegen.hpp"

namespace editor
{
ComponentHealthRegen::ComponentHealthRegen() {}
ComponentHealthRegen::~ComponentHealthRegen() {}

void ComponentHealthRegen::Serialize(icon7::ByteWriter &writer)
{
	ComponentCharacterSheet_HealthRegen regen;
	regen.cooldown = regenCooldown;
	regen.amount = regenAmount;
	reg::Registry::SerializePersistent(GameClientFrontend::singleton->realm,
									   regen, writer);
}

void ComponentHealthRegen::_bind_methods()
{
	REGISTER_PROPERTY(ComponentHealthRegen, regenAmount, Variant::Type::INT,
					  "regenAmount");
	REGISTER_PROPERTY(ComponentHealthRegen, regenCooldown, Variant::Type::INT,
					  "regenCooldown");
}
} // namespace editor
