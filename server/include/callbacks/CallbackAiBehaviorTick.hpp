#pragma once

#include "../ComponentCallbacks.hpp"

namespace named_callbacks
{
namespace registry_entries
{
using AiBehaviorTickFunctionType = void (*)(RealmServer *realm,
											uint64_t entityId);
struct AiBehaviorTick final
	: public EntryBase<AiBehaviorTick, AiBehaviorTickFunctionType> {
};
} // namespace registry_entries

template <>
std::shared_mutex Registry<registry_entries::AiBehaviorTick>::sharedMutex;

template <>
Registry<registry_entries::AiBehaviorTick>::Map
	Registry<registry_entries::AiBehaviorTick>::registry;
} // namespace named_callbacks
