#include "../include/callbacks/CallbackAiBehaviorTick.hpp"
#include "../include/callbacks/CallbackOnTriggerEnterExit.hpp"
#include "../include/callbacks/CallbackOnUse.hpp"

namespace named_callbacks
{
template <>
Registry<registry_entries::OnUse>::Map
	Registry<registry_entries::OnUse>::registry =
		Registry<registry_entries::OnUse>::Map{};

template <>
std::shared_mutex Registry<registry_entries::OnUse>::sharedMutex = {};

template <>
Registry<registry_entries::OnTriggerEnterExit>::Map
	Registry<registry_entries::OnTriggerEnterExit>::registry =
		Registry<registry_entries::OnTriggerEnterExit>::Map{};

template <>
std::shared_mutex Registry<registry_entries::OnTriggerEnterExit>::sharedMutex =
	{};

template <>
Registry<registry_entries::AiBehaviorTick>::Map
	Registry<registry_entries::AiBehaviorTick>::registry =
		Registry<registry_entries::AiBehaviorTick>::Map{};

template <>
std::shared_mutex Registry<registry_entries::AiBehaviorTick>::sharedMutex = {};
} // namespace named_callbacks
