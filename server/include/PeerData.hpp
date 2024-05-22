#pragma once

#include <string>

#include <flecs.h>

#include <icon7/Peer.hpp>

class RealmServer;

enum PeerState : uint32_t
{
	WAITING_FOR_USERNAME,
	FETCHING_USER_DATA,
	INVALID_USER_NAME,
	
	CONNECTED,
	DISCONNECTED,
	
	PLAYING_IN_REALM,
	DISCONNECTING_FROM_REALM,
	DISCONNECTING,
};

class PeerData
{
public:
	std::string userName;
	std::weak_ptr<icon7::Peer> peer;
	std::weak_ptr<RealmServer> realm;
	std::atomic<uint32_t> peerState = 0;
	uint64_t entityId = 0;
};
