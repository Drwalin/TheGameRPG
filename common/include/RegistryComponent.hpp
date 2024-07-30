#pragma once

#include <map>
#include <string>
#include <vector>

#include <flecs.h>

#include <icon7/Forward.hpp>

#include "Realm.hpp"

namespace reg
{

class ComponentConstructorBase
{
public:
	ComponentConstructorBase(std::string name, std::string fullName)
		: name(name), fullName(fullName)
	{
	}
	virtual ~ComponentConstructorBase() {}

	virtual void
	DeserializePersistentEntityComponent(class Realm *realm,
										 flecs::entity entity,
										 icon7::ByteReader &reader) const = 0;
	virtual bool
	SerializePersistentEntityComponent(class Realm *realm, flecs::entity entity,
									   icon7::ByteWriter &writer) const = 0;

	virtual void
	DeserializeTemporalEntityComponent(class Realm *realm, flecs::entity entity,
									   icon7::ByteReader &reader) const = 0;
	virtual bool
	SerializeTemporalEntityComponent(class Realm *realm, flecs::entity entity,
									 icon7::ByteWriter &writer) const = 0;

	virtual bool HasComponent(flecs::entity entity) const = 0;

	const std::string name = "";
	const std::string fullName = "";
};

class Registry
{
	Registry();
	~Registry();

public:
	static Registry &Singleton();

	template <typename T> void RegisterComponent(std::string name);

	template <typename T>
	static void SerializePersistent(class Realm *realm, const T &component,
									icon7::ByteWriter &writer);
	template <typename T>
	static void SerializePersistent(class Realm *realm, uint64_t entityId,
									icon7::ByteWriter &writer)
	{
		flecs::entity entity = realm->Entity(entityId);
		const T *component = entity.get<T>();
		if (component) {
			SerializePersistent<T>(realm, *component, writer);
		}
	}
	bool DeserializePersistentEntityComponent(class Realm *realm,
											  flecs::entity entity,
											  icon7::ByteReader &reader);
	void DeserializePersistentAllEntityComponents(class Realm *realm,
												  flecs::entity entity,
												  icon7::ByteReader &reader);
	void SerializePersistentEntity(class Realm *realm, flecs::entity entity,
								   icon7::ByteWriter &writer) const;

	template <typename T>
	static void SerializeTemporal(class Realm *realm, const T &component,
								  icon7::ByteWriter &writer);

	template <typename T>
	static void SerializeTemporal(class Realm *realm, uint64_t entityId,
								  icon7::ByteWriter &writer)
	{
		flecs::entity entity = realm->Entity(entityId);
		const T *component = entity.get<T>();
		if (component) {
			SerializeTemporal<T>(realm, *component, writer);
		}
	}

	bool DeserializeTemporalEntityComponent(class Realm *realm,
											flecs::entity entity,
											icon7::ByteReader &reader);
	void DeserializeTemporalAllEntityComponents(class Realm *realm,
												flecs::entity entity,
												icon7::ByteReader &reader);

	void SerializeTemporalEntity(class Realm *realm, flecs::entity entity,
								 icon7::ByteWriter &writer) const;

private:
	std::vector<ComponentConstructorBase *> components;
	std::map<std::string, ComponentConstructorBase *> nameToComponent;
};
} // namespace reg
