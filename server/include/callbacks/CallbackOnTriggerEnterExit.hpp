#pragma once

#include "../ComponentCallbacks.hpp"

namespace named_callbacks
{
namespace registry_entries
{
using OnTriggerEnterExitFunctionType = void (*)(RealmServer *realm,
												uint64_t entityId,
												uint64_t triggerId);
struct OnTriggerEnterExit final
	: public EntryBase<OnTriggerEnterExit, OnTriggerEnterExitFunctionType> {
};
} // namespace registry_entries

template <>
std::shared_mutex Registry<registry_entries::OnTriggerEnterExit>::sharedMutex;

template <>
Registry<registry_entries::OnTriggerEnterExit>::Map
	Registry<registry_entries::OnTriggerEnterExit>::registry;
} // namespace named_callbacks
