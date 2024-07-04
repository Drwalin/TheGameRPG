#pragma once

#include "../../common/include/EntityComponents.hpp"

#include "../../ICon7/include/icon7/Peer.hpp"

struct ComponentPlayerConnectionPeer {
	std::shared_ptr<icon7::Peer> peer;

	ComponentPlayerConnectionPeer(std::shared_ptr<icon7::Peer> peer)
		: peer(peer)
	{
	}

	DEFAULT_CONSTRUCTORS_AND_MOVE(ComponentPlayerConnectionPeer, MV(peer));
};
