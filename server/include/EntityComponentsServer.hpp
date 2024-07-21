#pragma once

#include <memory>

#include "../../common/include/ComponentsUtility.hpp"

#include <icon7/Forward.hpp>

struct ComponentPlayerConnectionPeer {
	std::shared_ptr<icon7::Peer> peer;

	ComponentPlayerConnectionPeer(std::shared_ptr<icon7::Peer> peer)
		: peer(peer)
	{
	}

	DEFAULT_CONSTRUCTORS_AND_MOVE(ComponentPlayerConnectionPeer, MV(peer));
};
