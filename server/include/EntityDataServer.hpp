#pragma once

#include "PeerData.hpp"
#include "../../ICon7-godot-client/ICon7/include/icon7/Debug.hpp"

struct EntityPlayerConnectionPeer {
	std::shared_ptr<icon7::Peer> peer;
};
