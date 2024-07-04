#include "../include/ComponentCallbackRegistry.hpp"

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
} // namespace named_callbacks
