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
}
}
