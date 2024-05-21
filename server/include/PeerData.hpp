#pragma once

#include <string>

#include <flecs.h>

#include <icon7/Peer.hpp>

class RealmServer;

class PeerData
{
public:
	std::string userName;
	std::shared_ptr<icon7::Peer> peer;
	std::weak_ptr<RealmServer> realm;
	uint64_t entityId = 0;
};
