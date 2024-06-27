#include "../include/ComponentCallbackRegistry.hpp"

namespace named_callbacks
{
template <>
Registry<registry_entries::OnUse>::Map
	Registry<registry_entries::OnUse>::registry =
		Registry<registry_entries::OnUse>::Map{};

template <>
std::shared_mutex Registry<registry_entries::OnUse>::sharedMutex = {};
} // namespace named_callbacks
