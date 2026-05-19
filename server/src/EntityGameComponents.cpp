#include "../../common/include/RegistryComponent.inl.hpp"
#include "../../common/include/ComponentCharacterSheet.hpp"

#include "../include/ComponentCallbackRegistry.hpp"

#include "../include/RealmServer.hpp"
#include "../include/callbacks/CallbackAiBehaviorTick.hpp"
#include "../include/callbacks/CallbackOnTriggerEnterExit.hpp"
#include "../include/callbacks/CallbackOnUse.hpp"

#include "../include/EntityGameComponents.hpp"

int RegisterEntityComponents(flecs::world &ecs);
int RegisterEntityComponentsServer(flecs::world &ecs);

int RegisterEntityGameComponents(flecs::world &ecs)
{
	RegisterEntityComponents(ecs);
	RegisterEntityComponentsServer(ecs);

	REGISTER_COMPONENT_FOR_ECS_WORLD(ecs, ComponentSpawner, "CSPAWNER");
	REGISTER_COMPONENT_FOR_ECS_WORLD(ecs, ComponentOnUse, "TCBONU");
	REGISTER_COMPONENT_FOR_ECS_WORLD(ecs, ComponentSingleDoorTransformStates,
									 "SDST");
	REGISTER_COMPONENT_FOR_ECS_WORLD(ecs, ComponentTeleport, "TELEPORT");
	REGISTER_COMPONENT_FOR_ECS_WORLD(ecs, ComponentTrigger, "TRIGGER");
	REGISTER_COMPONENT_FOR_ECS_WORLD(ecs, ComponentAITick, "AITICK");
	REGISTER_COMPONENT_NO_SERIALISATION(ecs, TagPrivateEntity);

	reg::ComponentConstructor<ComponentCharacterSheet_HealthRegen>::singleton
		->callbackDeserializePersistent =
		[](class Realm *realm, flecs::entity entity,
		   ComponentCharacterSheet_HealthRegen *hp) {
			hp->lastTimestamp += realm->timer.currentTick;
		};
	reg::ComponentConstructor<ComponentCharacterSheet_HealthRegen>::singleton
		->callbackSerializePersistent =
		[](class Realm *realm, ComponentCharacterSheet_HealthRegen *hp) {
			hp->lastTimestamp -= realm->timer.currentTick;
		};

	reg::ComponentConstructor<ComponentCharacterSheet_AttackCooldown>::singleton
		->callbackDeserializePersistent =
		[](class Realm *realm, flecs::entity entity,
		   ComponentCharacterSheet_AttackCooldown *hp) {
			hp->lastTimestamp += realm->timer.currentTick;
		};
	reg::ComponentConstructor<ComponentCharacterSheet_AttackCooldown>::singleton
		->callbackSerializePersistent =
		[](class Realm *realm, ComponentCharacterSheet_AttackCooldown *hp) {
			hp->lastTimestamp -= realm->timer.currentTick;
		};

	reg::ComponentConstructor<ComponentSpawner>::singleton
		->callbackDeserializePersistent = [](class Realm *realm,
											 flecs::entity entity,
											 ComponentSpawner *spawner) {
		spawner->lastSpawnedTimestamp += realm->timer.currentTick;
	};
	reg::ComponentConstructor<ComponentSpawner>::singleton
		->callbackSerializePersistent = [](class Realm *realm,
										   ComponentSpawner *spawner) {
		spawner->lastSpawnedTimestamp -= realm->timer.currentTick;
	};

	return 0;
}

void ComponentTrigger::Tick(int64_t entityId, RealmServer *realm)
{
	std::vector<uint64_t> entities;
	realm->collisionWorld.TriggerTestBoxForCharacters(realm->Entity(entityId),
													  entities);

	std::unordered_set<uint64_t> old;
	std::swap(old, entitiesInside);
	entitiesInside.insert(entities.begin(), entities.end());

	if (realm->timer.currentTick > tickUntilIgnore) {
		for (auto id : entities) {
			if (old.count(id) == 0) {
				if (onEnter) {
					if (realm->IsEntityAlive(id)) {
						onEnter->Call(realm, id, entityId);
					}
				}
			}
		}

		for (auto id : old) {
			if (entitiesInside.count(id) == 0) {
				if (onExit) {
					if (realm->IsEntityAlive(id)) {
						onExit->Call(realm, id, entityId);
					}
				}
			}
		}
	}
}

void ComponentOnUse::serialize(icon7::ByteReaderBase &s)
{
	entry->Deserialize(&entry, s);
}
void ComponentOnUse::serialize(icon7::ByteWriterBase &s)
{
	entry->Serialize(&entry, s);
}

BITSCPP_BYTESTREAM_OP_SYMMETRIC_DEFINITIONS(ComponentSingleDoorTransformStates,
											{
												s.op(transformClosed);
												s.op(transformOpen);
											});

BITSCPP_BYTESTREAM_OP_SYMMETRIC_DEFINITIONS(ComponentTeleport, {
	s.op(realmName);
	s.op(position);
});

inline void ComponentTrigger::serialize(icon7::ByteReaderBase &s)
{
	onEnter->Deserialize(&onEnter, s);
	onExit->Deserialize(&onExit, s);
}
inline void ComponentTrigger::serialize(icon7::ByteWriterBase &s) const
{
	onEnter->Serialize(&onEnter, s);
	onExit->Serialize(&onExit, s);
}

inline void ComponentAITick::serialize(icon7::ByteReaderBase &s)
{
	aiTick->Deserialize(&aiTick, s);
}
inline void ComponentAITick::serialize(icon7::ByteWriterBase &s) const
{
	aiTick->Serialize(&aiTick, s);
}

BITSCPP_BYTESTREAM_OP_SYMMETRIC_DEFINITIONS(ComponentSpawner, {
	s.op(maxAmount);
	s.op(spawnCooldown);
	s.op(spawnRadius);
	s.op(radiusToCheckAmountEntities);
	s.op(lastSpawnedTimestamp);
	s.op(prefabsData);
	s.op(prefabsOffset);
});
