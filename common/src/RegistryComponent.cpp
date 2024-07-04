#include "bitscpp/Endianness.hpp"

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

void Registry::DeserializeEntityComponent(flecs::entity entity,
										  icon7::ByteReader &reader)
{
	std::string n;
	reader.op(n);
	auto it = nameToComponent.find(n);
	if (it == nameToComponent.end()) {
		LOG_FATAL("Trying to deserialize non existing component with name: %s",
				  n.c_str());
		return;
	}
	it->second->DeserializeEntityComponent(entity, reader);
}

void Registry::DeserializeAllEntityComponents(flecs::entity entity,
											  icon7::ByteReader &reader)
{
	uint16_t count = 0;
	reader.op(count);
	for (int i = 0; i < count; ++i) {
		DeserializeEntityComponent(entity, reader);
	}
}

void Registry::SerializeEntity(flecs::entity entity,
							   icon7::ByteWriter &writer) const
{
	uint32_t offset = writer.GetSize();
	uint16_t count = 0;
	writer.op(count);
	for (auto c : components) {
		if (c->SerializeEntityComponent(entity, writer)) {
			++count;
		}
	}

	count = bitscpp::HostToNetworkUint<uint16_t>(count);
	writer.Buffer().data()[offset] = ((uint8_t *)&count)[0];
	writer.Buffer().data()[offset + 1] = ((uint8_t *)&count)[1];
}
} // namespace reg
