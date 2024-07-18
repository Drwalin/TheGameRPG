#include "../../common/include/RegistryComponent.inl.hpp"
#include "../include/RealmServer.hpp"

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
