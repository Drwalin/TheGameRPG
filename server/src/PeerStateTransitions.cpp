#include "../include/PeerData.hpp"
#include "../include/RealmServer.hpp"
#include "../include/ServerCore.hpp"
#include "../include/ClientRpcProxy.hpp"

#include "../include/PeerStateTransitions.hpp"

namespace peer_transitions
{
void OnReceivedLogin(icon7::Peer *peer, const std::string &username)
{
	PeerData *data = ((PeerData *)(peer->userPointer));
	if (data->userName != "" || username == "" || data->peer.expired()) {
		// TODO: re-login is not available
		LOG_INFO("Login failed: '%s' != \"\"  ||  '%s' == \"\"  ||  %s == true",
				 data->userName.c_str(), username.c_str(),
				 data->peer.expired() ? "true" : "false");
		ClientRpcProxy::LoginFailed(peer);
		return;
	} else {
		LOG_INFO("Player login successfull: '%s'", username.c_str());
	}
	data->userName = username;
	auto core = ((ServerCore *)(peer->host->userPointer));
	// TODO: get player data from database and call core->ConnectPeerToRealm
	core->ConnectPeerToRealm(peer, core->spawnRealm);
}
} // namespace peer_transitions
