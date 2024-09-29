#include <icon7/ByteReader.hpp>
#include <random>

#include "../../common/include/RegistryComponent.hpp"

#include "../include/RealmServer.hpp"
#include "../include/GameLogic.hpp"

#ifndef M_PI
#define M_PI 3.141592653589793238462643
#endif

namespace GameLogic
{
void Spawner(RealmServer *realm, flecs::entity spawnerEntity,
			 ComponentSpawner &spawner,
			 const ComponentStaticTransform &transform)
{
	if (spawner.prefabsData.empty() || spawner.prefabsOffset.size() <= 1) {
		return;
	}
	if (spawner.lastSpawnedTimestamp + spawner.spawnCooldown >=
		realm->timer.currentTick) {
		return;
	}
	spawner.lastSpawnedTimestamp = realm->timer.currentTick;

	std::vector<uint64_t> entities;
	realm->collisionWorld.TestForEntitiesSphere(
		transform.pos, spawner.radiusToCheckAmountEntities, &entities,
		CollisionWorld::FILTER_CHARACTER);

	int counter = 0;
	for (uint64_t id : entities) {
		flecs::entity entity = realm->Entity(id);
		if (entity.is_valid() && entity.is_alive()) {
			if (entity.has<ComponentAITick>()) {
				++counter;
			}
		}
	}

	if (counter < spawner.maxAmount) {
		int i = rand() % (spawner.prefabsOffset.size() - 1);
		icon7::ByteBuffer buffer(spawner.prefabsOffset[i] + 256);
		buffer.append(spawner.prefabsData.data() + spawner.prefabsOffset[i],
					  spawner.prefabsOffset[i + 1] - spawner.prefabsOffset[i]);
		icon7::ByteReader reader(std::move(buffer), 0);

		flecs::entity spawnedEntity = realm->Entity(realm->NewEntity());

		reg::Registry::Singleton().DeserializePersistentAllEntityComponents(
			realm, spawnedEntity, reader);

		glm::vec3 pos{0, 0, 0};

		static std::random_device rd;
		static std::mt19937_64 mt(rd());
		std::normal_distribution<float> dist(0.0, 0.2f);
		std::uniform_real_distribution<float> dist2pi(0.0, 2.0 * M_PI);
		do {
			pos.x = dist(mt);
			pos.z = dist(mt);
		} while (glm::dot(pos, pos) > 1.0);
		pos.x *= spawner.spawnRadius;
		pos.z *= spawner.spawnRadius;
		pos += transform.pos;

		glm::vec3 hitPoint;
		if (realm->collisionWorld.RayTestFirstHitTerrain(
				pos, pos + glm::vec3(0, -1000, 0), &hitPoint, nullptr,
				nullptr)) {
			hitPoint.y += 1.0;
			if (pos.y > hitPoint.y) {
				pos = hitPoint;
			}
		}

		float yRot = dist2pi(mt);
		realm->TrueDeferWhenNeeded([realm, pos, spawnedEntity, yRot]() {
			if (auto _ms =
					spawnedEntity
						.get<ComponentLastAuthoritativeMovementState>()) {
				auto ms = *_ms;
				ms.oldState.pos = pos;
				ms.oldState.onGround = false;
				ms.oldState.timestamp = realm->timer.currentTick;
				ms.oldState.rot.y += yRot;
				spawnedEntity.set<ComponentLastAuthoritativeMovementState>(ms);
				spawnedEntity.set<ComponentMovementState>(ms.oldState);
			} else if (auto _ms = spawnedEntity.get<ComponentMovementState>()) {
				auto ms = *_ms;
				ms.pos = pos;
				ms.onGround = false;
				ms.timestamp = realm->timer.currentTick;
				ms.rot.y += yRot;
				spawnedEntity.set<ComponentLastAuthoritativeMovementState>(ms);
				spawnedEntity.set<ComponentMovementState>(ms);
			} else if (auto _ts =
						   spawnedEntity.get<ComponentStaticTransform>()) {
				auto ts = *_ts;
				ts.pos = pos;
				spawnedEntity.set<ComponentStaticTransform>(ts);
			} else {
				LOG_INFO("Spawned entity from spawner does not have "
						 "ComponentLastAuthoritativeMovementState nor "
						 "ComponentMovementState nor "
						 "ComponentStaticTransform.");
			}
		});
	}
}
} // namespace GameLogic
