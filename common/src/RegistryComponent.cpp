#include <icon7/ByteReader.hpp>
#include <icon7/ByteWriter.hpp>
#include <icon7/Debug.hpp>

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
	it->second->DeserializePersistentEntityComponent(realm, entity, reader);
	return true;
}

void Registry::DeserializePersistentAllEntityComponents(
	class Realm *realm, flecs::entity entity, icon7::ByteReader &reader)
{
	while (reader.get_remaining_bytes() > 1) {
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
		auto bp = ce.get<_InternalComponent_ComponentConstructorBasePointer>();
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
	it->second->DeserializeTemporalEntityComponent(realm, entity, reader);
	return true;
}

void Registry::DeserializeTemporalAllEntityComponents(class Realm *realm,
													  flecs::entity entity,
													  icon7::ByteReader &reader)
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
		auto bp = ce.get<_InternalComponent_ComponentConstructorBasePointer>();
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
