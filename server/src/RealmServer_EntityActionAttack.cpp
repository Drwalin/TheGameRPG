#define GLM_ENABLE_EXPERIMENTAL
#include "../../thirdparty/Collision3D/SpatialPartitioning/glm/glm/gtx/vector_angle.hpp"
#include "../../thirdparty/Collision3D/SpatialPartitioning/glm/glm/gtx/quaternion.hpp"

#include <icon7/Peer.hpp>
#include <icon7/Flags.hpp>
#include <icon7/Debug.hpp>

#include "../../common/include/EntitySystems.hpp"
#include "../../common/include/ComponentCharacterSheet.hpp"

#include "../include/ClientRpcProxy.hpp"
#include "../include/EntityNetworkingSystems.hpp"

#include "../include/RealmServer.hpp"

void RealmServer::Attack(uint64_t instigatorId,
						 ComponentLastAuthoritativeMovementState state,
						 uint64_t targetId, glm::vec3 targetPos,
						 int64_t attackType, int64_t attackId, int64_t argInt)
{
	if (argInt != 0) {
		return;
	}

	int32_t dmg = attackId;

	bool inRange = false;

	if (instigatorId) {
		flecs::entity entityInstigator = Entity(instigatorId);
		if (entityInstigator.is_alive()) {
			auto atck_ =
				entityInstigator.get<ComponentCharacterSheet_AttackCooldown>();
			if (atck_) {
				auto atck = *atck_;
				if (atck.lastTimestamp + atck.baseCooldown >=
					timer.currentTick) {
					return;
				} else {
					auto modelName = entityInstigator.get<ComponentModelName>();
					if (modelName) {
						ClientRpcProxy::Broadcast_PlayAnimation(
							this, instigatorId, *modelName, state, "attack_1",
							timer.currentTick);
					}

					atck.lastTimestamp = timer.currentTick;
					entityInstigator
						.set<ComponentCharacterSheet_AttackCooldown>(atck);
				}
			}

			auto str_ =
				entityInstigator.get<ComponentCharacterSheet_Strength>();
			if (str_) {
				auto str = *str_;
				dmg += str.strength;
			}

			if (auto range =
					entityInstigator.get<ComponentCharacterSheet_Ranges>()) {
				glm::vec3 eyes = state.oldState.pos;
				if (auto shape = entityInstigator.get<ComponentShape>()) {
					eyes.y += shape->height;
				}

				if (glm::length2(eyes - targetPos) <=
					range->attackRange * range->attackRange) {
					inRange = true;
				}
			}
		}
	}

	if (targetId && inRange) {
		flecs::entity entityTarget = Entity(targetId);
		if (entityTarget.is_alive()) {

			auto ap_ = entityTarget.get<ComponentCharacterSheet_Protection>();
			if (ap_) {
				auto ap = *ap_;
				dmg -= ap.armorPoints;
			}

			if (dmg < 1) {
				dmg = 1;
			}

			auto hp_ = entityTarget.get<ComponentCharacterSheet_Health>();
			if (hp_) {
				ComponentCharacterSheet_Health hp = *hp_;
				if (hp.hp > 0) {
					hp.hp -= dmg;

					ClientRpcProxy::Broadcast_PlayFX(
						this, {"fx/Blood.tscn"},
						{targetPos, {1, 0, 0, 0}, {1, 1, 1}}, timer.currentTick,
						0, 5000);

					if (hp.hp <= 0) {
						hp.hp = 0;

						int64_t xp = 1;

						auto tlvl_ =
							entityTarget.get<ComponentCharacterSheet_LevelXP>();
						if (tlvl_) {
							auto tlvl = *tlvl_;
							xp = (tlvl.level) * (tlvl.level - 1);
							if (xp < 1) {
								xp = 1;
							}
						}

						if (instigatorId) {
							flecs::entity entityInstigator =
								Entity(instigatorId);
							if (entityInstigator.is_alive()) {
								auto ilvl_ =
									entityInstigator
										.get<ComponentCharacterSheet_LevelXP>();
								if (ilvl_) {
									auto ilvl = *ilvl_;

									ilvl.xp += xp;
									entityInstigator
										.set<ComponentCharacterSheet_LevelXP>(
											ilvl);
								}
							}
						}
					}
					entityTarget.set<ComponentCharacterSheet_Health>(hp);
				}
			}
		}
	}
}

void RealmServer::Attack(icon7::Peer *peer,
						 ComponentLastAuthoritativeMovementState state,
						 uint64_t targetId, glm::vec3 targetPos,
						 int64_t attackType, int64_t attackId, int64_t argInt)
{
	PeerData *data = ((PeerData *)(peer->userPointer));
	Attack(data->entityId, state, targetId, targetPos, attackType, attackId,
		   argInt);
}
