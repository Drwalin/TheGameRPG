#pragma once

#include <string>
#include <memory>

#include "../../thirdparty/Collision3D/SpatialPartitioning/glm/glm/ext/vector_float3.hpp"

#include <flecs.h>

#include <icon7/Forward.hpp>
#include <icon7/ByteBuffer.hpp>

class RealmServer;

enum PeerState : uint32_t {
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

	icon7::ByteBuffer storedEntityData;

	std::string nextRealm = "";
	bool useNextRealmPosition = false;
	glm::vec3 nextRealmPosition;
};
