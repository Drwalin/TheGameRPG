#include "../include/RegistryComponent.hpp"

namespace reg
{
Registry::Registry() {}
Registry::~Registry() {}
Registry &Registry::Singleton()
{
	static Registry reg;
	return reg;
}

bool Registry::DeserializeEntityComponent(flecs::entity entity,
										  icon7::ByteReader &reader)
{
	std::string n;
	reader.op(n);
	if (n == "") {
		return false;
	}
	auto it = nameToComponent.find(n);
	if (it == nameToComponent.end()) {
		LOG_FATAL("Trying to deserialize non existing component with name: %s",
				  n.c_str());
		return false;
	}
	it->second->DeserializeEntityComponent(entity, reader);
	return true;
}

void Registry::DeserializeAllEntityComponents(flecs::entity entity,
											  icon7::ByteReader &reader)
{
	while (reader.get_remaining_bytes() > 1) {
		if (DeserializeEntityComponent(entity, reader) == false) {
			break;
		}
	}
}

void Registry::SerializeEntity(flecs::entity entity,
							   icon7::ByteWriter &writer) const
{
	for (auto c : components) {
		c->SerializeEntityComponent(entity, writer);
	}
	writer.op("");
}
} // namespace reg
