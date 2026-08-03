#include "../../../ICon7/include/icon7/Debug.hpp"
#include "../../../ICon7/include/icon7/Peer.hpp"

#include "../../include/ServerCore.hpp"
#include "../../include/PeerData.hpp"

#include "../../include/commands/TeleportPlayer.hpp"

void CommandTeleportPlayer::Execute()
{
	auto it = serverCore->usernameToPeer.find(userName);
	if (it == serverCore->usernameToPeer.end()) {
		LOG_INFO("No username %s found to teleport", userName.c_str());
		return;
	}
	auto data = it->second;
	if (data == nullptr) {
		LOG_WARN("No data for username named `%s`", userName.c_str());
		return;
	}
	if (data) {
		data->nextRealm = realmName;
		data->nextRealmPosition = position;
		data->useNextRealmPosition = true;
		serverCore->ConnectPeerToRealm(data->peerHandle);
	}
}
