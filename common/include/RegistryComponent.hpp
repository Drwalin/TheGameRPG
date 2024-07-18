#pragma once

#include <map>
#include <string>
#include <vector>

#include <flecs.h>

#include <icon7/ByteReader.hpp>
#include <icon7/ByteWriter.hpp>
#include <icon7/Debug.hpp>

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
	DeserializeEntityComponent(flecs::entity entity,
							   icon7::ByteReader &reader) const = 0;
	virtual bool SerializeEntityComponent(flecs::entity entity,
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
	static void Serialize(const T &component, icon7::ByteWriter &writer);

	template <typename T>
	static void Serialize(class Realm *realm, uint64_t entityId,
						  icon7::ByteWriter &writer)
	{
		flecs::entity entity = realm->Entity(entityId);
		const T *component = entity.get<T>();
		if (component) {
			Serialize<T>(*component, writer);
		}
	}

	bool DeserializeEntityComponent(flecs::entity entity,
									icon7::ByteReader &reader);
	void DeserializeAllEntityComponents(flecs::entity entity,
										icon7::ByteReader &reader);

	void SerializeEntity(flecs::entity entity, icon7::ByteWriter &writer) const;

private:
	std::vector<ComponentConstructorBase *> components;
	std::map<std::string, ComponentConstructorBase *> nameToComponent;
};
} // namespace reg
