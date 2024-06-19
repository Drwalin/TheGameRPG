#pragma once

#include <map>
#include <string>

#include <vector>

#include <flecs.h>

#include <icon7/ByteReader.hpp>
#include <icon7/ByteWriter.hpp>
#include <icon7/Debug.hpp>

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

template <typename T>
class ComponentConstructor : public ComponentConstructorBase
{
	static uint32_t __dummy;

public:
	ComponentConstructor(std::string name, std::string fullName)
		: ComponentConstructorBase(name, fullName)
	{
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
		const T *component = entity.get<T>();
		if (component) {
			Serialize(*component, writer);
			return true;
		}
		return false;
	}

	static void Serialize(const T &component, icon7::ByteWriter &writer)
	{
		writer.op(singleton.name);
		writer.op(component);
	}

	virtual bool HasComponent(flecs::entity entity) const override
	{
		return entity.has<T>();
	}

	static ComponentConstructor<T> singleton;
};

class Registry
{
	Registry();
	~Registry();

public:
	static Registry &Singleton();

	template <typename T> void RegisterComponent(std::string name)
	{
		if (nameToComponent.count(name) != 0) {
			LOG_FATAL("Component with name %s already exists", name.c_str());
			return;
		}
		ComponentConstructorBase *com = &ComponentConstructor<T>::singleton;
		components.push_back(com);
		nameToComponent.insert({name, com});
	}

	template <typename T>
	static void Serialize(const T &component, icon7::ByteWriter &writer)
	{
		ComponentConstructor<T>::Serialize(component, writer);
	}

	void DeserializeEntityComponent(flecs::entity entity,
									icon7::ByteReader &reader);
	void DeserializeAllEntityComponents(flecs::entity entity,
										icon7::ByteReader &reader);

	void SerializeEntity(flecs::entity entity, icon7::ByteWriter &writer) const;

private:
	std::vector<ComponentConstructorBase *> components;
	std::map<std::string, ComponentConstructorBase *> nameToComponent;
};
} // namespace reg

#define GAME_REGISTER_ECS_COMPONENT_STATIC(COMPONENT, NAME)                    \
	template <>                                                                \
	reg::ComponentConstructor<COMPONENT>                                       \
		reg::ComponentConstructor<COMPONENT>::singleton =                      \
			reg::ComponentConstructor<COMPONENT>(NAME, #COMPONENT);                        \
	template <>                                                                \
	uint32_t reg::ComponentConstructor<COMPONENT>::__dummy =                   \
		(reg::Registry::Singleton().RegisterComponent<COMPONENT>(NAME), 0);
