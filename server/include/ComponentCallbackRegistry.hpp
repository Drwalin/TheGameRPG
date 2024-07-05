#pragma once

#include <chrono>
#include <shared_mutex>
#include <memory>
#include <string>
#include <unordered_map>

#include <bitscpp/ByteWriter.hpp>
#include <bitscpp/ByteReader.hpp>
#include <icon7/Debug.hpp>

#include "SharedObject.hpp"

class RealmServer;

namespace named_callbacks
{
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
			LOG_FATAL(
				"Trying to re-register existing named collback with differing "
				"full and short names. Named callback registry entries of `%s` "
				"and `%s` differ:   `%s`:{'%s' , '%s'} != `%s`:{'%s' , '%s'}",
				fullName.c_str(), shortName.c_str(), fullName.c_str(),
				it1->second->fullName.c_str(), it1->second->shortName.c_str(),
				shortName.c_str(), it1->second->shortName.c_str(),
				it1->second->shortName.c_str());
			return;
		}

		if (it1 != registry.end()) {
			it1->second->setTimestamp = std::chrono::system_clock::now();
			it1->second->callback = cb;
			it1->second->sharedObject = sharedObject;
			LOG_TRACE("Re-registering callback '%s' -> '%s'", fullName.c_str(),
					  shortName.c_str());
		} else {
			T *ptr = new T{fullName, shortName,
						   std::chrono::system_clock::now(), cb, sharedObject};
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
			return nullptr;
		}
		return ret;
	}

private:
	static Map registry;
	static std::shared_mutex sharedMutex;
};
} // namespace named_callbacks
