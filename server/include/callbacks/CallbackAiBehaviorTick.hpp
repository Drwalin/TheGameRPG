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
} // namespace named_callbacks
