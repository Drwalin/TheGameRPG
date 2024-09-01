#include <icon7/Peer.hpp>
#include <icon7/Flags.hpp>
#include <icon7/Debug.hpp>

#include "../../common/include/EntitySystems.hpp"
#include "../../common/include/ComponentCharacterSheet.hpp"

#include "../include/ClientRpcProxy.hpp"
#include "../include/EntityNetworkingSystems.hpp"

#include "../include/RealmServer.hpp"

void RealmServer::Attack(uint64_t instigatorId, ComponentMovementState state,
						 uint64_t targetId, glm::vec3 targetPos,
						 int64_t attackType, int64_t attackId, int64_t argInt)
{
	if (argInt != 0) {
		return;
	}

	LOG_INFO("Send play attack animation");

	int32_t dmg = attackId;

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
		}
	}

	if (targetId) {
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
					LOG_INFO("Play damage received animation or effect");
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
									entityInstigator.set(ilvl);
								}
							}
						}
					}
					entityTarget.set(hp);
				}
			}
		}
	}
}

void RealmServer::Attack(icon7::Peer *peer, ComponentMovementState state,
						 uint64_t targetId, glm::vec3 targetPos,
						 int64_t attackType, int64_t attackId, int64_t argInt)
{
	PeerData *data = ((PeerData *)(peer->userPointer));
	Attack(data->entityId, state, targetId, targetPos, attackType, attackId,
		   argInt);
}
