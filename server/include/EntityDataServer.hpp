#pragma once

#include "PeerData.hpp"

struct EntityPlayerConnectionPeer {
	std::shared_ptr<icon7::Peer> peer;
	inline EntityPlayerConnectionPeer() = default;
	EntityPlayerConnectionPeer(EntityPlayerConnectionPeer &) = default;
	EntityPlayerConnectionPeer(EntityPlayerConnectionPeer &&) = default;
	EntityPlayerConnectionPeer(const EntityPlayerConnectionPeer &) = default;
	EntityPlayerConnectionPeer &
	operator=(EntityPlayerConnectionPeer &) = default;
	EntityPlayerConnectionPeer &
	operator=(EntityPlayerConnectionPeer &&) = default;
	EntityPlayerConnectionPeer &
	operator=(const EntityPlayerConnectionPeer &) = default;
	inline EntityPlayerConnectionPeer(std::shared_ptr<icon7::Peer> peer)
		: peer(peer)
	{
	}
};
