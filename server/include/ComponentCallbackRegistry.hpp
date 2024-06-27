#pragma once

#include <chrono>
#include <shared_mutex>
#include <memory>
#include <atomic>
#include <string>
#include <unordered_map>

#include <bitscpp/ByteWriter.hpp>
#include <bitscpp/ByteReader.hpp>
#include <icon7/Debug.hpp>

#include "SharedObject.hpp"

class RealmServer;

namespace named_callbacks
{
template <typename TFinal, typename TCb> struct EntryBase {
	using FunctionType = TCb;

	const std::string fullName;
	const std::string shortName;
	std::chrono::system_clock::time_point setTimestamp;
	std::atomic<TCb> callback;
	std::shared_ptr<SharedObject> sharedObject;

	inline static int Set(const std::string &fullName,
						  const std::string &shortName, FunctionType cb,
						  std::shared_ptr<SharedObject> sharedObject);
	inline static TFinal *Get(const std::string &name);

	template <typename... TArgs> inline void Call(TArgs &&...args)
	{
		callback.load()(std::forward<TArgs>(args)...);
	}
};

template <typename T> struct Registry {
	using Map = std::unordered_map<std::string, T *>;
	using FunctionType = typename T::FunctionType;

	static void Set(const std::string &fullName, const std::string &shortName,
					FunctionType cb, std::shared_ptr<SharedObject> sharedObject)
	{
		sharedMutex.lock();
		auto it1 = registry.find(fullName);
		auto it2 = registry.find(shortName);

		if (it1 != it2) {
			LOG_FATAL("Named callback registry entries of `%s` and `%s` differ:"
					  "   `%s`:{'%s' , '%s'} != `%s`:{'%s' , '%s'}",
					  fullName.c_str(), shortName.c_str(), fullName.c_str(),
					  it1->second->fullName.c_str(),
					  it1->second->shortName.c_str(), shortName.c_str(),
					  it1->second->shortName.c_str(),
					  it1->second->shortName.c_str(), sharedObject);
			return;
		}

		if (it1 != registry.end()) {
			it1->second->setTimestamp = std::chrono::system_clock::now();
			it1->second->callback = cb;
			LOG_TRACE("Re-registering callback '%s' -> '%s'", fullName.c_str(),
					  shortName.c_str());
		} else {
			T *ptr = new T{fullName, shortName,
						   std::chrono::system_clock::now(), cb};
			registry[fullName] = ptr;
			registry[shortName] = ptr;
			LOG_TRACE("Registering callback '%s' -> '%s'", fullName.c_str(),
					  shortName.c_str());
		}
		sharedMutex.unlock();
	}

	static T *Get(const std::string &name)
	{
		T *ret = nullptr;
		sharedMutex.lock_shared();
		auto it = registry.find(name);
		if (it != registry.end()) {
			ret = it->second;
		}
		sharedMutex.unlock_shared();
		if (ret == nullptr) {
			LOG_ERROR("No callback named: %s", name.c_str());
			exit(13);
		}
		return ret;
	}

private:
	static Map registry;
	static std::shared_mutex sharedMutex;
};

template <typename TFinal, typename TCb>
inline int
EntryBase<TFinal, TCb>::Set(const std::string &fullName,
							const std::string &shortName, TCb cb,
							std::shared_ptr<SharedObject> sharedObject)
{
	Registry<TFinal>::Set(fullName, shortName, cb, sharedObject);
	return 0;
}

template <typename TFinal, typename TCb>
inline TFinal *EntryBase<TFinal, TCb>::Get(const std::string &name)
{
	return Registry<TFinal>::Get(name);
}

template <typename TFinal, typename TCb> struct ComponentCallbackBase {
	TFinal *entry = nullptr;

	bitscpp::ByteReader<true> &__ByteStream_op(bitscpp::ByteReader<true> &s)
	{
		std::string name;
		s.op(name);
		if (s.is_valid()) {
			entry = TFinal::Get(name);
		} else {
			entry = nullptr;
		}
		return s;
	}
	template <typename BT>
	inline bitscpp::ByteWriter<BT> &__ByteStream_op(bitscpp::ByteWriter<BT> &s)
	{
		if (entry) {
			s.op(entry->shortName);
		} else {
			s.op("");
		}
		return s;
	}

	template <typename... TArgs> inline void Call(TArgs &&...args)
	{
		entry->Call(std::forward<TArgs>(args)...);
	}
};

namespace registry_entries
{
using OnUseFunctionType = void (*)(RealmServer *realm, uint64_t instigatorId,
								   uint64_t receiverId,
								   const std::string &context);
struct OnUse final : public EntryBase<OnUse, OnUseFunctionType> {
};

#define REGISTER_NAMED_CALLBACK(TYPE, FULL_NAME, SHORT_NAME, FUNC,             \
								SHARED_OBJECT)                                 \
	TYPE::Set(FULL_NAME, SHORT_NAME, FUNC, SHARED_OBJECT)
} // namespace registry_entries
} // namespace named_callbacks
