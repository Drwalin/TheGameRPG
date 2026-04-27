#include "../../ICon7/bitscpp/include/bitscpp/ByteReader_v2.hpp"
#include "../../ICon7/include/icon7/ByteWriter.hpp"
#include "../../ICon7/include/icon7/Debug.hpp"

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

bool Registry::DeserializePersistentEntityComponent(class Realm *realm,
													flecs::entity entity,
													bitscpp::v2::ByteReader &reader)
{
	std::string n;
	uint8_t peek = reader.get_buffer()[reader.get_offset()];
	LOG_INFO("Trying deserialization of a component (reader %u / %u): %s ; next type: %u <- 0x%2.2X",
			reader.get_offset(),
			reader.get_remaining_bytes(),
			reader.is_valid() ? "valid" : "error",
			(uint32_t)reader.get_next_type(),
			(uint32_t)peek
			);
	if (reader.is_next_string()) {
		reader.op(n);
	}
	if (n == "") {
		int64_t i = 69696969;
	LOG_INFO("Trying deserialization of a component -- empty end of object (reader %u / %u): %s : %u ; int_instead_of_string: %li",
			reader.get_offset(),
			reader.get_remaining_bytes(),
			reader.is_valid() ? "valid" : "error",
			reader.get_errors(),
			i
			);
		LOG_INFO("Empty end of object");
		return false;
	}
	LOG_INFO("Trying deserialize (reader %u): %s", reader.get_offset(), n.c_str());
	auto it = nameToComponent.find(n);
	if (it == nameToComponent.end()) {
		LOG_FATAL("Trying to deserialize non existing component with name: %s",
				  n.c_str());
		return false;
	}
	it->second->DeserializePersistentEntityComponent(realm, entity, reader);
	return true;
}

void Registry::DeserializePersistentAllEntityComponents(
	class Realm *realm, flecs::entity entity, bitscpp::v2::ByteReader &reader)
{
	int i=0;
	while (reader.get_remaining_bytes() > 1) {
		++i;
		LOG_INFO("i = %i", i);
		if (DeserializePersistentEntityComponent(realm, entity, reader) ==
			false) {
			break;
		}
	}
}

void Registry::SerializePersistentEntity(class Realm *realm,
										 flecs::entity entity,
										 icon7::ByteWriter &writer) const
{
	auto type = entity.type();
	for (int i = 0; i < type.count(); ++i) {
		flecs::id cid = type.get(i);
		if (!cid.is_entity()) {
			continue;
		}
		flecs::entity ce = cid.entity();
		auto bp = ce.try_get<_InternalComponent_ComponentConstructorBasePointer>();
		if (bp == nullptr) {
			LOG_ERROR("Component %lu (%s) of an entity %lu does not have "
					  "_InternalComponent_ComponentConstructorBasePointer",
					  cid.entity().id(), cid.str().c_str(), entity.id());
			continue;
		}
		if (bp->ptr == nullptr) {
			continue;
		}
		bp->ptr->SerializePersistentEntityComponent(realm, entity, writer);
	}
	writer.op("");
}

bool Registry::DeserializeTemporalEntityComponent(class Realm *realm,
												  flecs::entity entity,
												  bitscpp::v2::ByteReader &reader)
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
	it->second->DeserializeTemporalEntityComponent(realm, entity, reader);
	return true;
}

void Registry::DeserializeTemporalAllEntityComponents(class Realm *realm,
													  flecs::entity entity,
													  bitscpp::v2::ByteReader &reader)
{
	while (reader.get_remaining_bytes() > 1) {
		if (DeserializeTemporalEntityComponent(realm, entity, reader) ==
			false) {
			break;
		}
	}
}

void Registry::SerializeTemporalEntity(class Realm *realm, flecs::entity entity,
									   icon7::ByteWriter &writer) const
{
	auto type = entity.type();
	for (int i = 0; i < type.count(); ++i) {
		flecs::id cid = type.get(i);
		if (!cid.is_entity()) {
			continue;
		}
		flecs::entity ce = cid.entity();
		auto bp = ce.try_get<_InternalComponent_ComponentConstructorBasePointer>();
		if (bp == nullptr) {
			LOG_ERROR("Component %lu (%s) of an entity %lu does not have "
					  "_InternalComponent_ComponentConstructorBasePointer",
					  cid.entity().id(), cid.str().c_str(), entity.id());
			continue;
		}
		if (bp->ptr == nullptr) {
			continue;
		}
		bp->ptr->SerializeTemporalEntityComponent(realm, entity, writer);
	}
	writer.op("");
}
} // namespace reg
