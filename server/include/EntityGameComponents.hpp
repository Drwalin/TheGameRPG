#pragma once

#include <unordered_set>

#include "ComponentCallbacks.hpp"

#include "../../common/include/Tick.hpp"
#include "../../common/include/EntityComponents.hpp"

namespace named_callbacks
{
namespace registry_entries
{
struct AiBehaviorTick;
struct OnTriggerEnterExit;
struct OnUse;
} // namespace registry_entries
} // namespace named_callbacks

struct ComponentOnUse {
	named_callbacks::registry_entries::OnUse *entry = nullptr;

	BITSCPP_BYTESTREAM_OP_DECLARATIONS()
	DEFAULT_CONSTRUCTORS_AND_MOVE(ComponentOnUse, MV(entry));
};

struct ComponentSingleDoorTransformStates {
	ComponentStaticTransform transformClosed;
	ComponentStaticTransform transformOpen;

	BITSCPP_BYTESTREAM_OP_DECLARATIONS()
	DEFAULT_CONSTRUCTORS_AND_MOVE(ComponentSingleDoorTransformStates,
								  {MV(transformClosed) MV(transformOpen)});
};

struct ComponentTrigger {
	named_callbacks::registry_entries::OnTriggerEnterExit *onEnter = nullptr;
	named_callbacks::registry_entries::OnTriggerEnterExit *onExit = nullptr;

	std::unordered_set<uint64_t> entitiesInside = {};
	Tick tickUntilIgnore = {0};

	void Tick(int64_t entityId, class RealmServer *realm);

	BITSCPP_BYTESTREAM_OP_DECLARATIONS()
	DEFAULT_CONSTRUCTORS_AND_MOVE(ComponentTrigger,
								  {MV(onEnter) MV(onExit) MV(entitiesInside)
									   MV(tickUntilIgnore)});
};

struct ComponentTeleport {
	std::string realmName = "";
	glm::vec3 position;

	BITSCPP_BYTESTREAM_OP_DECLARATIONS()
	DEFAULT_CONSTRUCTORS_AND_MOVE(ComponentTeleport,
								  {MV(realmName) MV(position)});
};

struct ComponentAITick {
	named_callbacks::registry_entries::AiBehaviorTick *aiTick = nullptr;

	BITSCPP_BYTESTREAM_OP_DECLARATIONS()
	DEFAULT_CONSTRUCTORS_AND_MOVE(ComponentAITick, MV(aiTick));
};

struct ComponentSpawner {
	int64_t maxAmount = 10;
	int64_t spawnCooldown = 3000;
	float spawnRadius = 32.0f;
	float radiusToCheckAmountEntities = 128.0f;
	Tick lastSpawnedTimestamp = {0};
	std::vector<uint8_t> prefabsData;
	std::vector<uint32_t> prefabsOffset;

	BITSCPP_BYTESTREAM_OP_DECLARATIONS()
	DEFAULT_CONSTRUCTORS_AND_MOVE(
		ComponentSpawner,
		{MV(maxAmount) MV(spawnCooldown) MV(spawnRadius)
			 MV(radiusToCheckAmountEntities) MV(lastSpawnedTimestamp)
				 MV(prefabsData) MV(prefabsOffset)});
};

// Prevents from sending entity with this component to players
struct TagPrivateEntity {};
