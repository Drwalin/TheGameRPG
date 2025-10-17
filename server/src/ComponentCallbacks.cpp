#include "../include/callbacks/CallbackAiBehaviorTick.hpp"
#include "../include/callbacks/CallbackOnTriggerEnterExit.hpp"
#include "../include/callbacks/CallbackOnUse.hpp"

namespace named_callbacks
{
#define DEFINE_NAMED_CALLBACK_METHODS(NAMED_CALLBACK_TYPE)                     \
	template <>                                                                \
	std::shared_mutex &                                                        \
	Registry<registry_entries::NAMED_CALLBACK_TYPE>::SharedMutex()             \
	{                                                                          \
		static std::shared_mutex mutex;                                        \
		return mutex;                                                          \
	}                                                                          \
	template <>                                                                \
	Registry<registry_entries::NAMED_CALLBACK_TYPE>::Map &                     \
	Registry<registry_entries::NAMED_CALLBACK_TYPE>::registry()                \
	{                                                                          \
		static Registry<registry_entries::NAMED_CALLBACK_TYPE>::Map map;       \
		return map;                                                            \
	}

DEFINE_NAMED_CALLBACK_METHODS(AiBehaviorTick);
DEFINE_NAMED_CALLBACK_METHODS(OnUse);
DEFINE_NAMED_CALLBACK_METHODS(OnTriggerEnterExit);
} // namespace named_callbacks
