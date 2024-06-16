#pragma once

#include <set>
#include <map>
#include <string>

#include <vector>

#include <flecs.h>

#include <icon7/ByteReader.hpp>
#include <icon7/ByteWriter.hpp>
#include <icon7/Debug.hpp>

namespace reg
{

class SpecializedRegistry;

class ComponentConstructorBase
{
public:
	ComponentConstructorBase(std::string name) : name(name) {}
	virtual ~ComponentConstructorBase() {}

	virtual void
	DeserializeEntityComponent(flecs::entity entity,
							   icon7::ByteReader &reader) const = 0;
	virtual bool SerializeEntityComponent(flecs::entity entity,
										  icon7::ByteWriter &writer) const = 0;
	virtual bool HasComponent(flecs::entity entity) const = 0;

	const std::string name = "";
	uint16_t id = 0;
};

template <typename T>
class ComponentConstructor : public ComponentConstructorBase
{
	static uint32_t __dummy;

public:
	ComponentConstructor(std::string name) : ComponentConstructorBase(name) {}
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
		writer.op(id);
		const T *component = entity.get<T>();
		if (component) {
			writer.op(*component);
			return true;
		}
		return false;
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
		uint16_t id = GetNextComponentId();
		ComponentConstructorBase *com = &ComponentConstructor<T>::singleton;
		if (nameToId.count(name) != 0) {
			LOG_FATAL("Component with name %s already exists", name.c_str());
			return;
		}
		components.push_back(com);
		nameToId.insert({name, id});
		idToComponent.insert({id, com});
	}

	uint16_t GetNextComponentId() const;
	void SetComponentId(std::string name, uint16_t newId);

	void WriteComponentsConfiguration(icon7::ByteWriter &writer);
	void RestoreComponentsConfiguration(icon7::ByteReader &reader);

	void DeserializeEntityComponent(flecs::entity entity,
									icon7::ByteReader &reader);
	void DeserializeAllEntityComponents(flecs::entity entity,
										icon7::ByteReader &reader);

	void SerializeEntity(flecs::entity entity, icon7::ByteWriter &writer) const;

private:
	std::vector<ComponentConstructorBase *> components;
	std::map<std::string, uint16_t> nameToId;
	std::map<uint16_t, ComponentConstructorBase *> idToComponent;
};

class SpecializedRegistry
{
public:
	SpecializedRegistry();
	~SpecializedRegistry();

	template <typename T> void RegisterComponent()
	{
		components.insert(&ComponentConstructor<T>::singleton);
	}

	void SerializeEntity(flecs::entity entity, icon7::ByteWriter &writer) const;

private:
	std::set<ComponentConstructorBase *> components;
};

template <typename T>
void RegisterComponentIntoSpecializedRegistries(
	std::vector<SpecializedRegistry *> regs)
{
	for (auto r : regs) {
		r->RegisterComponent<T>();
	}
}

extern SpecializedRegistry entityPublicRegistry;
extern SpecializedRegistry entityPrivateRegistry;

} // namespace reg

#define GAME_REGISTER_ECS_COMPONENT_STATIC(COMPONENT, ...)                     \
	template <>                                                                \
	reg::ComponentConstructor<COMPONENT>                                       \
		reg::ComponentConstructor<COMPONENT>::singleton =                      \
			reg::ComponentConstructor<COMPONENT>(#COMPONENT);                  \
	template <>                                                                \
	uint32_t reg::ComponentConstructor<COMPONENT>::__dummy =                   \
		(reg::Registry::Singleton().RegisterComponent<COMPONENT>(#COMPONENT),  \
		 reg::RegisterComponentIntoSpecializedRegistries<COMPONENT>(           \
			 {__VA_ARGS__}),                                                   \
		 0);
