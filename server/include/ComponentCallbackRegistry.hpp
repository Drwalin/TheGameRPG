#pragma once

#include <shared_mutex>
#include <memory>
#include <string>
#include <unordered_map>

#include "../../ICon7/include/icon7/Time.hpp"
#include "../../ICon7/include/icon7/Debug.hpp"

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

		if (it1 != registry.end() && it2 != registry.end()) {
			if (it1->second != it2->second) {
				sharedMutex.unlock();
				LOG_FATAL("Trying to re-register existing named callback with "
						  "differing "
						  "full and short names. Named callback registry "
						  "entries of `%s` "
						  "and `%s` differ:   (%p):{'%s' , '%s'} != (%p):{'%s' "
						  ", '%s'}",
						  fullName.c_str(), shortName.c_str(), it1->second,
						  it1->second->fullName.c_str(),
						  it1->second->shortName.c_str(), it2->second,
						  it2->second->fullName.c_str(),
						  it2->second->shortName.c_str());
				return;
			}
		} else if (it1 != registry.end()) {
			sharedMutex.unlock();
			LOG_FATAL("Trying to register or re-register named callback with "
					  "existing full name but with different short name."
					  "Trying to register {'%s' , '%s'} but found "
					  "(%p):{'%s' , '%s'}",
					  fullName.c_str(), shortName.c_str(), it1->second,
					  it1->second->fullName.c_str(),
					  it1->second->shortName.c_str());
			return;
		} else if (it2 != registry.end()) {
			sharedMutex.unlock();
			LOG_FATAL("Trying to register or re-register named callback with "
					  "existing short name but with different full name."
					  "Trying to register {'%s' , '%s'} but found "
					  "(%p):{'%s' , '%s'}",
					  fullName.c_str(), shortName.c_str(), it2->second,
					  it2->second->fullName.c_str(),
					  it2->second->shortName.c_str());
			return;
		}

		if (it1 != registry.end()) {
			it1->second->setTimestamp = icon7::time::GetTemporaryTimestamp();
			it1->second->callback = cb;
			it1->second->sharedObject = sharedObject;
			LOG_TRACE("Re-registering callback '%s' -> '%s'", fullName.c_str(),
					  shortName.c_str());
		} else {
			T *ptr = new T{fullName, shortName,
						   icon7::time::GetTemporaryTimestamp(), cb, sharedObject};
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
			LOG_INFO("No callback named: %s", name.c_str());
			return nullptr;
		}
		return ret;
	}

private:
	static Map registry;
	static std::shared_mutex sharedMutex;
};
} // namespace named_callbacks
