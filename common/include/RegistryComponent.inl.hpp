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

	virtual void
	DeserializeEntityComponent(flecs::entity entity,
							   icon7::ByteReader &reader) const override
	{
		T component;
		reader.op(component);
		if (reader.is_valid()) {
			entity.set<T>(component);
		}
	}

	virtual bool
	SerializeEntityComponent(flecs::entity entity,
							 icon7::ByteWriter &writer) const override
	{
		if (entity.has<T>()) {
			const T *component = entity.get<T>();
			if (component) {
				Serialize(*component, writer);
				return true;
			}
		}
		return false;
	}

	static void Serialize(const T &component, icon7::ByteWriter &writer)
	{
		writer.op(singleton->name);
		writer.op(component);
	}

	virtual bool HasComponent(flecs::entity entity) const override
	{
		return entity.has<T>();
	}

	static ComponentConstructor<T> *singleton;
};

template <typename T>
class ComponentConstructorWithCallbackDeserialize
	: public ComponentConstructor<T>
{
public:
	ComponentConstructorWithCallbackDeserialize(
		std::string name, std::string fullName,
		std::function<void(flecs::entity, T *)> callback)
		: ComponentConstructor<T>(name, fullName), callback(callback)
	{
	}
	virtual ~ComponentConstructorWithCallbackDeserialize() {}

	virtual void
	DeserializeEntityComponent(flecs::entity entity,
							   icon7::ByteReader &reader) const override
	{
		T component;
		reader.op(component);
		if (reader.is_valid()) {
			callback(entity, &component);
			entity.set<T>(std::move(component));
		}
	}

	std::function<void(flecs::entity, T *)> callback;
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
extern void Registry::Serialize(const T &component, icon7::ByteWriter &writer)
{
	ComponentConstructor<T>::Serialize(component, writer);
}
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
	void Registry::Serialize<COMPONENT>(const COMPONENT &component,            \
										icon7::ByteWriter &writer)             \
	{                                                                          \
		ComponentConstructor<COMPONENT>::Serialize(component, writer);         \
	}                                                                          \
	}

#define GAME_REGISTER_ECS_COMPONENT_STATIC_WITH_DESERIALIZE_CALLBACK(          \
	COMPONENT, NAME, CALLBACK)                                                 \
	namespace reg                                                              \
	{                                                                          \
	template <>                                                                \
	ComponentConstructor<COMPONENT>                                            \
		*ComponentConstructor<COMPONENT>::singleton =                          \
			new ComponentConstructorWithCallbackDeserialize<COMPONENT>(        \
				NAME, #COMPONENT, CALLBACK);                                   \
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
	void Registry::Serialize<COMPONENT>(const COMPONENT &component,            \
										icon7::ByteWriter &writer)             \
	{                                                                          \
		ComponentConstructor<COMPONENT>::Serialize(component, writer);         \
	}                                                                          \
	}
