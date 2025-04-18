#include <icon7/Debug.hpp>
#include <icon7/Host.hpp>
#include <icon7/Peer.hpp>
#include <icon7/CoroutineHelper.hpp>
#include <icon7/CommandExecutionQueue.hpp>

#include "../include/PeerData.hpp"
#include "../include/RealmServer.hpp"
#include "../include/ServerCore.hpp"
#include "../include/ClientRpcProxy.hpp"
#include "../include/FileOperations.hpp"

#include "../include/PeerStateTransitions.hpp"

namespace peer_transitions
{
icon7::CoroutineSchedulable OnReceivedLogin(ServerCore *serverCore,
											icon7::Peer *peer,
											const std::string &username)
{
	PeerData *data = ((PeerData *)(peer->userPointer));
	if (data->realm.lock().get() != nullptr || data->userName != "" ||
		username == "" || data->peer.expired()) {
		// TODO: re-login is not available
		LOG_INFO("Login failed: '%s' != \"\"  ||  '%s' == \"\"  ||  %s == true",
				 data->userName.c_str(), username.c_str(),
				 data->peer.expired() ? "true" : "false");
		ClientRpcProxy::LoginFailed(
			peer, "Invalid login or connection or you are already logged in");
		co_return;
	} else {
		auto it = serverCore->usernameToPeer.find(username);
		if (it == serverCore->usernameToPeer.end()) {
			serverCore->usernameToPeer[username] = peer->shared_from_this();
			LOG_INFO("Player login successfull: '%s'", username.c_str());
			ClientRpcProxy::LoginSuccessfull(peer);
		} else {
			LOG_INFO("User already in-game: `%s`", username.c_str());
			ClientRpcProxy::LoginFailed(
				peer, "User with this username is already logged in");
			co_return;
		}
	}
	data->userName = username;
	auto core = ((ServerCore *)(peer->host->userPointer));

	std::weak_ptr<icon7::Peer> weakPeer = peer->weak_from_this();

	{
		// TODO: read it from database or something
		data->nextRealm = core->spawnRealm;

		// TODO: replace with async delayed scheduled read from database or file
		//       and then call core->ConnectPeerToRealm(peer)
		// use something like:
		//     co_await ScheduleReadFromDatabase(...);

		// Load player data from file

		std::string filePrefix;
		if (serverCore->configStorage.GetString(
				"config.users_directory_storage.prefix", &filePrefix)) {
		}
		if (FileOperations::ReadFile(filePrefix + username,
									 &(data->storedEntityData))) {
			if (data->storedEntityData.size()) {
				data->nextRealm = (char *)data->storedEntityData.data();
			}
		}
	}

	core->ConnectPeerToRealm(peer);
}
} // namespace peer_transitions
