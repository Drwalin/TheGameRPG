#include "../include/PeerData.hpp"
#include "../include/RealmServer.hpp"
#include "../include/ServerCore.hpp"

#include "../include/PeerStateTransitions.hpp"

namespace peer_transitions
{
void OnReceivedLogin(icon7::Peer *peer, const std::string &username)
{
	PeerData *data = ((PeerData *)(peer->userPointer));
	
	
	
	
	
	
	
	
	
	auto core = ((ServerCore *)(peer->host->userPointer));
	// TODO: get player data from database and call core->ConnectPeerToRealm
	core->ConnectPeerToRealm(peer, core->spawnRealm);
	
	
}
}
