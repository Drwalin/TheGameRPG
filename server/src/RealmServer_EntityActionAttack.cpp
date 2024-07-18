#include <icon7/RPCEnvironment.hpp>
#include <icon7/Peer.hpp>
#include <icon7/Flags.hpp>
#include <icon7/Debug.hpp>

#include "../../common/include/EntitySystems.hpp"

#include "../include/ClientRpcProxy.hpp"
#include "../include/EntityNetworkingSystems.hpp"
#include "../include/EntityGameComponents.hpp"

#include "../include/RealmServer.hpp"

void RealmServer::Attack(uint64_t instigatorId, ComponentMovementState state,
						 uint64_t targetId, glm::vec3 targetPos,
						 int64_t attackType, int64_t attackId,
						 const std::string &argStr, int64_t argInt)
{
	
	
	
	
	
	
	
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
