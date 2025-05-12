#include "../../../server/include/EntityGameComponents.hpp"
#include "../../../server/include/callbacks/CallbackAiBehaviorTick.hpp"
#include "../../../common/include/RegistryComponent.hpp"

#include "../GameClientFrontend.hpp"

#include "EditorConfig.hpp"
#include "ComponentAITick.hpp"

namespace editor
{
ComponentAITick::ComponentAITick() {}
ComponentAITick::~ComponentAITick() {}

void ComponentAITick::Serialize(icon7::ByteWriter &writer)
{
	UtilityFunctions::print("Saving aitick");
	std::string onAiTick = aiTickName.utf8().ptr();
	named_callbacks::registry_entries::AiBehaviorTick aiTickEntry{
		onAiTick, onAiTick, {}, nullptr, nullptr};
	::ComponentAITick aiTick;
	aiTick.aiTick = &aiTickEntry;
	reg::Registry::SerializePersistent(GameClientFrontend::singleton->realm,
									   aiTick, writer);
}

void ComponentAITick::_bind_methods()
{
	REGISTER_PROPERTY(ComponentAITick, aiTickName, Variant::Type::STRING,
					  "AI Tick");
}
} // namespace editor
