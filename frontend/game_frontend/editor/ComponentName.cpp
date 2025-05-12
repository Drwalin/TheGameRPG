#include "../../../common/include/RegistryComponent.hpp"

#include "../GameClientFrontend.hpp"

#include "EditorConfig.hpp"
#include "ComponentName.hpp"

namespace editor
{
ComponentName::ComponentName() {}
ComponentName::~ComponentName() {}

void ComponentName::Serialize(icon7::ByteWriter &writer)
{
	reg::Registry::SerializePersistent(GameClientFrontend::singleton->realm,
									   ::ComponentName{name.utf8().ptr()},
									   writer);
}

void ComponentName::_bind_methods()
{
	REGISTER_PROPERTY(ComponentName, name, Variant::Type::STRING,
					  "Name");
}
} // namespace editor
