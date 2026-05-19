#pragma once

#include <string>
#include <memory>

#include "../../thirdparty/Collision3D/SpatialPartitioning/glm/glm/ext/vector_float3.hpp"

#include "../../ICon7/include/icon7/Forward.hpp"
#include "../../ICon7/include/icon7/ByteBuffer.hpp"

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

class PeerData : std::enable_shared_from_this<PeerData>
{
public:
	std::string userName;
	std::shared_ptr<icon7::Peer> peer;
	icon7::PeerHandle peerHandle;
	std::weak_ptr<RealmServer> realm;
	std::atomic<uint32_t> peerState = 0;
	uint64_t entityId = 0;

	icon7::ByteBufferWritable storedEntityData;

	std::string nextRealm = "";
	bool useNextRealmPosition = false;
	glm::vec3 nextRealmPosition;
};
