#include "../../../server/include/callbacks/CallbackOnTriggerEnterExit.hpp"
#include "../../../server/include/EntityGameComponents.hpp"
#include "../../../common/include/RegistryComponent.hpp"

#include "../GameClientFrontend.hpp"

#include "EditorConfig.hpp"
#include "ComponentTrigger.hpp"

namespace editor
{
ComponentTrigger::ComponentTrigger() {}
ComponentTrigger::~ComponentTrigger() {}

void ComponentTrigger::Serialize(icon7::ByteWriter &writer)
{
	UtilityFunctions::print("Saving trigger");
	std::string onEnter = onTriggerEnter.utf8().ptr();
	named_callbacks::registry_entries::OnTriggerEnterExit onTriggerEnter{
		onEnter, onEnter, {}, nullptr, nullptr};
	std::string onExit = onTriggerExit.utf8().ptr();
	named_callbacks::registry_entries::OnTriggerEnterExit onTriggerExit{
		onExit, onExit, {}, nullptr, nullptr};
	::ComponentTrigger onTrigger;
	onTrigger.onEnter = &onTriggerEnter;
	onTrigger.onExit = &onTriggerExit;
	reg::Registry::SerializePersistent(GameClientFrontend::singleton->realm,
									   onTrigger, writer);
}

void ComponentTrigger::_bind_methods()
{
	REGISTER_PROPERTY(ComponentTrigger, onTriggerEnter, Variant::Type::STRING,
					  "OnEnter");
	REGISTER_PROPERTY(ComponentTrigger, onTriggerExit, Variant::Type::STRING,
					  "OnExit");
}

String ComponentTrigger::get_onTriggerEnter() { return onTriggerEnter; }
void ComponentTrigger::set_onTriggerEnter(String v) { onTriggerEnter = v; }

String ComponentTrigger::get_onTriggerExit() { return onTriggerExit; }
void ComponentTrigger::set_onTriggerExit(String v) { onTriggerExit = v; }
} // namespace editor
