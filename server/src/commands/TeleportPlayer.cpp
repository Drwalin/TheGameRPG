#include "../../ICon7/include/icon7/Debug.hpp"
#include "../../ICon7/include/icon7/Peer.hpp"

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
	auto peer = it->second;
	PeerData *data = ((PeerData *)(peer->userPointer));
	if (data) {
		data->nextRealm = realmName;
		data->nextRealmPosition = position;
		data->useNextRealmPosition = true;
		serverCore->ConnectPeerToRealm(peer.get());
	} else {
		LOG_ERROR("Peer 0x%p does not have initialised userData", peer.get());
	}
}
