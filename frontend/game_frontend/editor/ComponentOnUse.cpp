#include "../../../common/include/RegistryComponent.hpp"
#include "../../../server/include/callbacks/CallbackOnUse.hpp"
#include "../../../server/include/EntityGameComponents.hpp"

#include "../GameClientFrontend.hpp"

#include "EditorConfig.hpp"
#include "ComponentOnUse.hpp"

namespace editor
{
ComponentOnUse::ComponentOnUse() {}
ComponentOnUse::~ComponentOnUse() {}

void ComponentOnUse::Serialize(icon7::ByteWriter &writer)
{
	UtilityFunctions::print("Saving onuse: ", OnUse, ", ", OnUse);
	std::string onUseCallbackName = OnUse.utf8().ptr();
	named_callbacks::registry_entries::OnUse onUseEntry{
		onUseCallbackName, onUseCallbackName, {}, nullptr, nullptr};
	::ComponentOnUse onUse;
	onUse.entry = &onUseEntry;
	reg::Registry::SerializePersistent(GameClientFrontend::singleton->realm,
									   onUse, writer);
}

void ComponentOnUse::_bind_methods()
{
	REGISTER_PROPERTY(ComponentOnUse, OnUse,
					  Variant::Type::STRING, "OnUseFunctionName");
}

String ComponentOnUse::get_OnUse() { return OnUse; }
void ComponentOnUse::set_OnUse(String v) { OnUse = v; }

} // namespace editor
