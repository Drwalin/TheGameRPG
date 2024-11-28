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
	// 	static uint32_t __dummy;

public:
	ComponentConstructor(std::string name, std::string fullName,
						 std::string c_fileName, int line)
		: ComponentConstructorBase(name, fullName, c_fileName, line)
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

public: // function pointer holders
	static void (*__PTR_HOLDER_1__)(class Realm *, const T &,
									icon7::ByteWriter &);
	static void (*__PTR_HOLDER_2__)(class Realm *, const T &,
									icon7::ByteWriter &);
};

template <typename T>
extern void Registry::RegisterComponent(const std::string name)
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
ComponentConstructor<T> *ComponentConstructor<T>::singleton = nullptr;
template <typename T>
void (*ComponentConstructor<T>::__PTR_HOLDER_1__)(
	class Realm *, const T &, icon7::ByteWriter &) = nullptr;
template <typename T>
void (*ComponentConstructor<T>::__PTR_HOLDER_2__)(
	class Realm *, const T &, icon7::ByteWriter &) = nullptr;
} // namespace reg

#define REGISTER_COMPONENT_FOR_ECS_WORLD(ECS, COMPONENT, NAME)                 \
	{                                                                          \
		flecs::entity ce = ECS.component<COMPONENT>();                         \
		if (reg::ComponentConstructor<COMPONENT>::singleton != nullptr) {      \
			if (reg::ComponentConstructor<COMPONENT>::singleton->FILE_NAME ==  \
					__FILE__ &&                                                \
				reg::ComponentConstructor<COMPONENT>::singleton->LINE ==       \
					__LINE__) {                                                \
			} else {                                                           \
				LOG_FATAL("Component %s is already initiated elsewhere with "  \
						  "name %s",                                           \
						  #COMPONENT, std::string(NAME).c_str());              \
			}                                                                  \
		} else {                                                               \
			auto ptr = new reg::ComponentConstructor<COMPONENT>(               \
				NAME, #COMPONENT, __FILE__, __LINE__);                         \
			reg::ComponentConstructor<COMPONENT>::singleton = ptr;             \
			reg::Registry::Singleton().RegisterComponent<COMPONENT>(NAME);     \
			reg::ComponentConstructor<COMPONENT>::__PTR_HOLDER_1__ =           \
				&reg::Registry::SerializePersistent;                           \
			reg::ComponentConstructor<COMPONENT>::__PTR_HOLDER_2__ =           \
				&reg::Registry::SerializeTemporal;                             \
		}                                                                      \
		auto ptr = reg::ComponentConstructor<COMPONENT>::singleton;            \
		ce.set<_InternalComponent_ComponentConstructorBasePointer>({ptr});     \
	}

#define REGISTER_COMPONENT_NO_SERIALISATION(ECS, COMPONENT)                    \
	ECS.component<COMPONENT>()                                                 \
		.set<_InternalComponent_ComponentConstructorBasePointer>({nullptr})
