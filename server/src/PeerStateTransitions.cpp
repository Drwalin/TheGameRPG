#include "../include/PeerData.hpp"
#include "../include/RealmServer.hpp"
#include "../include/ServerCore.hpp"
#include "../include/ClientRpcProxy.hpp"
#include "../include/FileOperations.hpp"

#include "../include/PeerStateTransitions.hpp"

namespace peer_transitions
{
void OnReceivedLogin(icon7::Peer *peer, const std::string &username)
{
	PeerData *data = ((PeerData *)(peer->userPointer));
	if (data->realm.lock().get() != nullptr || data->userName != "" ||
		username == "" || data->peer.expired()) {
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

	{
		// TODO: read it from database or something
		data->nextRealm = core->spawnRealm;

		// TODO: replace with async delayed scheduled read from database or file
		//       and then call core->ConnectPeerToRealm(peer)

		// Load player data from file
		
		if (FileOperations::ReadFile(std::string("users/" + username),
					&(data->storedEntityData))) {
			if (data->storedEntityData.size()) {
				data->nextRealm = (char *)data->storedEntityData.data();
			}
		}
	}

	core->ConnectPeerToRealm(peer);
}
} // namespace peer_transitions
