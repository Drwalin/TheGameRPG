#include <icon7/RPCEnvironment.hpp>
#include <icon7/Peer.hpp>
#include <icon7/Flags.hpp>
#include <icon7/Debug.hpp>

#include "../../common/include/EntitySystems.hpp"

#include "../include/ClientRpcProxy.hpp"
#include "../include/EntityNetworkingSystems.hpp"

#include "../include/RealmServer.hpp"

void RealmServer::Attack(uint64_t instigatorId, ComponentMovementState state,
						 uint64_t targetId, glm::vec3 targetPos,
						 int64_t attackType, int64_t attackId,
						 const std::string &argStr, int64_t argInt)
{
	if (targetId) {
		flecs::entity entity = Entity(targetId);
		if (entity.is_alive()) {
			auto ms = entity.get<ComponentLastAuthoritativeMovementState>();
			if (ms) {
				auto s = ms->oldState;

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
				entity.set<ComponentLastAuthoritativeMovementState>(ls);
			}
		}
	}
}

void RealmServer::Attack(icon7::Peer *peer, ComponentMovementState state,
						 uint64_t targetId, glm::vec3 targetPos,
						 int64_t attackType, int64_t attackId,
						 const std::string &argStr, int64_t argInt)
{
	PeerData *data = ((PeerData *)(peer->userPointer));
	Attack(data->entityId, state, targetId, targetPos, attackType, attackId,
		   argStr, argInt);
}
