#pragma once

#include <string>
#include <functional>

#include <flecs.h>

#include <icon7/ByteReader.hpp>
#include <icon7/ByteWriter.hpp>
#include <icon7/Debug.hpp>

#include "RegistryComponent.hpp"

namespace reg
{

template <typename T>
class ComponentConstructor : public ComponentConstructorBase
{
	static uint32_t __dummy;

public:
	ComponentConstructor(std::string name, std::string fullName)
		: ComponentConstructorBase(name, fullName)
	{
		LOG_TRACE("Registering component: %s -> %s", fullName.c_str(),
				  name.c_str());
	}
	virtual ~ComponentConstructor() {}

	virtual void DeserializePersistentEntityComponent(
		class Realm *realm, flecs::entity entity,
		icon7::ByteReader &reader) const override
	{
		T component;
		reader.op(component);
		if (reader.is_valid()) {
			if (callbackDeserializePersistent) {
				callbackDeserializePersistent(realm, entity, &component);
			}
			entity.set<T>(component);
		}
	}
	virtual bool
	SerializePersistentEntityComponent(class Realm *realm, flecs::entity entity,
									   icon7::ByteWriter &writer) const override
	{
		if (entity.has<T>()) {
			const T *component = entity.get<T>();
			if (component) {
				SerializePersistent(realm, *component, writer);
				return true;
			}
		}
		return false;
	}
	static void SerializePersistent(class Realm *realm, const T &component,
									icon7::ByteWriter &writer)
	{
		writer.op(singleton->name);
		if (singleton->callbackSerializePersistent) {
			T comp = component;
			singleton->callbackSerializePersistent(realm, &comp);
			writer.op(comp);
		} else {
			writer.op(component);
		}
	}

	virtual void
	DeserializeTemporalEntityComponent(class Realm *realm, flecs::entity entity,
									   icon7::ByteReader &reader) const override
	{
		T component;
		reader.op(component);
		if (reader.is_valid()) {
			if (callbackDeserializeTemporal) {
				callbackDeserializeTemporal(realm, entity, &component);
			}
			entity.set<T>(component);
		}
	}
	virtual bool
	SerializeTemporalEntityComponent(class Realm *realm, flecs::entity entity,
									 icon7::ByteWriter &writer) const override
	{
		if (entity.has<T>()) {
			const T *component = entity.get<T>();
			if (component) {
				SerializeTemporal(realm, *component, writer);
				return true;
			}
		}
		return false;
	}
	static void SerializeTemporal(class Realm *realm, const T &component,
								  icon7::ByteWriter &writer)
	{
		writer.op(singleton->name);
		if (singleton->callbackSerializeTemporal) {
			T comp = component;
			singleton->callbackSerializeTemporal(realm, &comp);
			writer.op(comp);
		} else {
			writer.op(component);
		}
	}

	virtual bool HasComponent(flecs::entity entity) const override
	{
		return entity.has<T>();
	}

public:
	std::function<void(class Realm *, flecs::entity, T *)>
		callbackDeserializePersistent;
	std::function<void(class Realm *, T *)> callbackSerializePersistent;
	std::function<void(class Realm *, flecs::entity, T *)>
		callbackDeserializeTemporal;
	std::function<void(class Realm *, T *)> callbackSerializeTemporal;

public:
	static ComponentConstructor<T> *singleton;
};

template <typename T> extern void Registry::RegisterComponent(std::string name)
{
	if (nameToComponent.count(name) != 0) {
		LOG_FATAL("Component with name %s already exists", name.c_str());
		return;
	}
	ComponentConstructorBase *com = ComponentConstructor<T>::singleton;
	components.push_back(com);
	nameToComponent.insert({name, com});
}

template <typename T>
extern void Registry::SerializePersistent(class Realm *realm,
										  const T &component,
										  icon7::ByteWriter &writer)
{
	ComponentConstructor<T>::SerializePersistent(realm, component, writer);
}

template <typename T>
extern void Registry::SerializeTemporal(class Realm *realm, const T &component,
										icon7::ByteWriter &writer)
{
	ComponentConstructor<T>::SerializeTemporal(realm, component, writer);
}

template <typename T>
ComponentConstructor<T> *ComponentConstructor<T>::singleton;
} // namespace reg

#define GAME_REGISTER_ECS_COMPONENT_STATIC(COMPONENT, NAME)                    \
	namespace reg                                                              \
	{                                                                          \
	template <>                                                                \
	ComponentConstructor<COMPONENT>                                            \
		*ComponentConstructor<COMPONENT>::singleton =                          \
			new ComponentConstructor<COMPONENT>(NAME, #COMPONENT);             \
	template <>                                                                \
	uint32_t ComponentConstructor<COMPONENT>::__dummy =                        \
		(Registry::Singleton().RegisterComponent<COMPONENT>(NAME), 0);         \
	extern auto                                                                \
		__##COMPONENT##_holder_of_serialize_deserialize(const COMPONENT &c,    \
														icon7::ByteWriter &w)  \
	{                                                                          \
		LOG_FATAL("This function cannot ever be called");                      \
		return &Registry::RegisterComponent<COMPONENT>;                        \
	}                                                                          \
	template <>                                                                \
	void Registry::SerializePersistent<COMPONENT>(class Realm * realm,         \
												  const COMPONENT &component,  \
												  icon7::ByteWriter &writer)   \
	{                                                                          \
		ComponentConstructor<COMPONENT>::SerializePersistent(realm, component, \
															 writer);          \
	}                                                                          \
	template <>                                                                \
	void Registry::SerializeTemporal<COMPONENT>(class Realm * realm,           \
												const COMPONENT &component,    \
												icon7::ByteWriter &writer)     \
	{                                                                          \
		ComponentConstructor<COMPONENT>::SerializeTemporal(realm, component,   \
														   writer);            \
	}                                                                          \
	}
