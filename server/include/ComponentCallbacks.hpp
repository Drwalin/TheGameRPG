#pragma once

#include <chrono>
#include <memory>
#include <atomic>
#include <string>

#include <bitscpp/ByteWriter.hpp>
#include <bitscpp/ByteReader.hpp>
#include <icon7/Debug.hpp>

#include "SharedObject.hpp"
#include "ComponentCallbackRegistry.hpp"

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
						  std::shared_ptr<SharedObject> sharedObject)
	{
		Registry<TFinal>::Set(fullName, shortName, cb, sharedObject);
		return 0;
	}

	inline static TFinal *Get(const std::string &name)
	{
		return Registry<TFinal>::Get(name);
	}

	template <typename... TArgs> inline void Call(TArgs &&...args)
	{
		TCb ptr = callback.load();
		if (ptr != nullptr) {
			LOG_ERROR("Non null callback: %s", fullName.c_str());
			callback.load()(std::forward<TArgs>(args)...);
		} else {
			LOG_ERROR("Callback `%s` is null", fullName.c_str());
		}
	}

	static inline void Deserialize(TFinal **cb, bitscpp::ByteReader<true> &s)
	{
		std::string name;
		s.op(name);
		if (s.is_valid()) {
			*cb = TFinal::Get(name);
		} else {
			*cb = nullptr;
		}
	}

	template <typename BT>
	static inline void Serialize(TFinal **cb, bitscpp::ByteWriter<BT> &s)
	{
		if (*cb) {
			s.op((*cb)->shortName);
		} else {
			s.op("");
		}
	}
};

namespace registry_entries
{
using OnUseFunctionType = void (*)(RealmServer *realm, uint64_t instigatorId,
								   uint64_t receiverId,
								   const std::string &context);
struct OnUse final : public EntryBase<OnUse, OnUseFunctionType> {
};

using OnTriggerEnterExitFunctionType = void (*)(RealmServer *realm,
												uint64_t entityId,
												uint64_t triggerId);
struct OnTriggerEnterExit final
	: public EntryBase<OnTriggerEnterExit, OnTriggerEnterExitFunctionType> {
};
} // namespace registry_entries
} // namespace named_callbacks
