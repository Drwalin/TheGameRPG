#pragma once

#include "../ComponentCallbacks.hpp"

namespace named_callbacks
{
namespace registry_entries
{
using OnUseFunctionType = void (*)(RealmServer *realm, uint64_t instigatorId,
								   uint64_t receiverId,
								   const std::string &context);
struct OnUse final : public EntryBase<OnUse, OnUseFunctionType> {
};
} // namespace registry_entries
} // namespace named_callbacks
