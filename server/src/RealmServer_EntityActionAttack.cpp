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
	
	if (targetId) {
		flecs::entity entityTarget = Entity(targetId);
		if (entityTarget.is_alive()) {
			auto ms =
				entityTarget.get<ComponentLastAuthoritativeMovementState>();
			if (ms) {
				auto s = ms->oldState;

				ExecuteMovementUpdate(targetId, &s);

				glm::vec3 dir =
					glm::normalize(targetPos -
								   (state.pos + glm::vec3(0, 1.5, 0))) *
					3.0f * ((attackId >> 1) + 1.0f);

				if (attackId % 2 == 1) {
					dir = -dir;
				}

				s.vel = dir;
				s.vel.y = 3.0f;
				ComponentLastAuthoritativeMovementState ls;
				ls.oldState = s;
				entityTarget.set<ComponentLastAuthoritativeMovementState>(ls);
			}

			if (attackId % 2 == 1) {
				if (instigatorId) {
					flecs::entity entityInstigator = Entity(instigatorId);
					if (entityInstigator.is_alive()) {
						auto hp_ = entityInstigator
									   .get<ComponentCharacterSheet_Health>();
						if (hp_) {
							ComponentCharacterSheet_Health hp = *hp_;
							if (hp.hp > 0) {
								hp.hp -= 1;
								entityInstigator.set(hp);
							}
						}
					}
				}
			} else {
				auto hp_ = entityTarget.get<ComponentCharacterSheet_Health>();
				if (hp_) {
					ComponentCharacterSheet_Health hp = *hp_;
					if (hp.hp > 0) {
						hp.hp -= 1;
						entityTarget.set(hp);
					}
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
