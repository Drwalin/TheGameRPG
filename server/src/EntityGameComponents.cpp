#include "../../common/include/RegistryComponent.inl.hpp"
#include "../../common/include/ComponentCharacterSheet.hpp"
#include "../include/RealmServer.hpp"

#include "../include/callbacks/CallbackAiBehaviorTick.hpp"
#include "../include/callbacks/CallbackOnTriggerEnterExit.hpp"
#include "../include/callbacks/CallbackOnUse.hpp"

#include "../include/EntityGameComponents.hpp"

GAME_REGISTER_ECS_COMPONENT_STATIC(ComponentOnUse, "TCBONU");
GAME_REGISTER_ECS_COMPONENT_STATIC(ComponentSingleDoorTransformStates, "SDST");
GAME_REGISTER_ECS_COMPONENT_STATIC(ComponentTeleport, "TELEPORT");
GAME_REGISTER_ECS_COMPONENT_STATIC(ComponentTrigger, "TRIGGER");
GAME_REGISTER_ECS_COMPONENT_STATIC(ComponentAITick, "AITICK");

int RegisterEntityComponents(flecs::world &ecs);
int RegisterEntityComponentsServer(flecs::world &ecs);

int RegisterEntityGameComponents(flecs::world &ecs)
{
	RegisterEntityComponents(ecs);
	RegisterEntityComponentsServer(ecs);
	ecs.component<ComponentOnUse>();
	ecs.component<ComponentSingleDoorTransformStates>();
	ecs.component<ComponentTeleport>();
	ecs.component<ComponentTrigger>();
	ecs.component<ComponentAITick>();

	reg::ComponentConstructor<ComponentCharacterSheet_HealthRegen>::singleton
		->callbackDeserializePersistent =
		[](class Realm *realm, flecs::entity entity,
		   ComponentCharacterSheet_HealthRegen *hp) {
			hp->lastTimestamp = realm->timer.currentTick;
		};

	reg::ComponentConstructor<ComponentCharacterSheet_AttackCooldown>::singleton
		->callbackDeserializePersistent =
		[](class Realm *realm, flecs::entity entity,
		   ComponentCharacterSheet_AttackCooldown *hp) {
			hp->lastTimestamp = realm->timer.currentTick;
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

bitscpp::ByteReader<true> &
ComponentOnUse::__ByteStream_op(bitscpp::ByteReader<true> &s)
{
	entry->Deserialize(&entry, s);
	return s;
}
inline bitscpp::ByteWriter<icon7::ByteBuffer> &
ComponentOnUse::__ByteStream_op(bitscpp::ByteWriter<icon7::ByteBuffer> &s)
{
	entry->Serialize(&entry, s);
	return s;
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

bitscpp::ByteReader<true> &
ComponentTrigger::__ByteStream_op(bitscpp::ByteReader<true> &s)
{
	onEnter->Deserialize(&onEnter, s);
	onExit->Deserialize(&onExit, s);
	return s;
}
inline bitscpp::ByteWriter<icon7::ByteBuffer> &
ComponentTrigger::__ByteStream_op(bitscpp::ByteWriter<icon7::ByteBuffer> &s)
{
	onEnter->Serialize(&onEnter, s);
	onExit->Serialize(&onExit, s);
	return s;
}

bitscpp::ByteReader<true> &
ComponentAITick::__ByteStream_op(bitscpp::ByteReader<true> &s)
{
	aiTick->Deserialize(&aiTick, s);
	return s;
}
inline bitscpp::ByteWriter<icon7::ByteBuffer> &
ComponentAITick::__ByteStream_op(bitscpp::ByteWriter<icon7::ByteBuffer> &s)
{
	aiTick->Serialize(&aiTick, s);
	return s;
}
