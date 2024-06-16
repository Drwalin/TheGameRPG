#pragma once

#include "../../ICon7/include/icon7/Peer.hpp"

struct ComponentPlayerConnectionPeer {
	std::shared_ptr<icon7::Peer> peer;

	inline ComponentPlayerConnectionPeer() = default;
	ComponentPlayerConnectionPeer(ComponentPlayerConnectionPeer &) = default;
	ComponentPlayerConnectionPeer(ComponentPlayerConnectionPeer &&) = default;
	ComponentPlayerConnectionPeer(const ComponentPlayerConnectionPeer &) =
		default;
	ComponentPlayerConnectionPeer &
	operator=(ComponentPlayerConnectionPeer &) = default;
	ComponentPlayerConnectionPeer &
	operator=(ComponentPlayerConnectionPeer &&) = default;
	ComponentPlayerConnectionPeer &
	operator=(const ComponentPlayerConnectionPeer &) = default;
	inline ComponentPlayerConnectionPeer(std::shared_ptr<icon7::Peer> peer)
		: peer(peer)
	{
	}
};
