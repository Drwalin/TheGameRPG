#include "../include/RegistryComponent.hpp"
#include "bitscpp/Endianness.hpp"

namespace reg
{
Registry::Registry() {}
Registry::~Registry() {}
Registry &Registry::Singleton()
{
	static Registry reg;
	return reg;
}

uint16_t Registry::GetNextComponentId() const
{
	for (uint16_t i = 1; i != 0; ++i) {
		if (idToComponent.find(i) == idToComponent.end()) {
			return i;
		}
	}
	LOG_FATAL("Out of type ids.");
	return 0;
}

void Registry::SetComponentId(std::string name, uint16_t newId)
{
	auto it = idToComponent.find(newId);
	if (it != idToComponent.end()) {
		SetComponentId(it->second->name, GetNextComponentId());
	}

	auto itName = nameToId.find(name);
	if (itName == nameToId.end()) {
		LOG_FATAL(
			"Trying to set entity component id to %u for unregistered type: %s",
			newId, name.c_str());
		return;
	}

	uint16_t oldId = itName->second;
	itName->second = newId;

	it = idToComponent.find(oldId);
	ComponentConstructorBase *com = it->second;
	idToComponent.erase(it);
	idToComponent.insert({newId, com});
	com->id = newId;
}

void Registry::WriteComponentsConfiguration(icon7::ByteWriter &writer)
{
	uint16_t size = components.size();
	writer.op(size);
	for (auto c : components) {
		writer.op(c->name);
		writer.op(c->id);
	}
}

void Registry::RestoreComponentsConfiguration(icon7::ByteReader &reader)
{
	uint16_t size = 0;
	std::string name;
	uint16_t id;
	reader.op(size);
	for (int i = 0; i < size; ++i) {
		name = "";
		reader.op(name);
		id = 0;
		reader.op(id);
		SetComponentId(name, id);
	}
}

void Registry::DeserializeEntityComponent(flecs::entity entity,
										  icon7::ByteReader &reader)
{
	uint16_t componentId = 0;
	reader.op(componentId);
	auto it = idToComponent.find(componentId);
	if (it == idToComponent.end()) {
		LOG_FATAL("Trying to deserialize non existing component with id: %u",
				  (uint32_t)componentId);
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

void SpecializedRegistry::SerializeEntity(flecs::entity entity,
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
