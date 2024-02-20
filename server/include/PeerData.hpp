#pragma once

#include <string>

#include <icon7/Peer.hpp>

class Realm;

class PeerData
{
public:
	std::string userName;
	std::shared_ptr<icon7::Peer> peer;
	Realm *realm = nullptr;
	uint64_t entityId = 0;
};
