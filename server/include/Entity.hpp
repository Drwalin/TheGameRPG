#pragma once

#include <cstdio>
#include <cinttypes>

#include <string>

#include <glm/glm.hpp>

#include <icon7/Peer.hpp>

namespace bitscpp {
	template<>
	inline ByteReader<true>& op<glm::vec3>(ByteReader<true>& s, glm::vec3& v) {
		s.op(v.x);
		s.op(v.y);
		s.op(v.z);
		return s;
	}
	template<>
	inline ByteWriter& op<glm::vec3>(ByteWriter& s, const glm::vec3& v) {
		s.op(v.x);
		s.op(v.y);
		s.op(v.z);
		return s;
	}
}

class Realm;

class Entity
{
public:
	
	inline Entity();
	inline ~Entity();
	
	void Init(Realm *realm, uint64_t entityId);
	
	void ConnectPeer(icon7::Peer *peer);
	inline void DisconnectPeer();
	
	void Update(uint64_t currentTick);
	
	void SolvePlayerInput(uint64_t tick, glm::vec3 pos, glm::vec3 vel,
						  glm::vec3 forward);
	
	void SetModel(const std::string &modelName, float height, float width);
	
public:
	Realm *realm;
	
	uint64_t entityId;
	
	uint64_t lastUpdateTick;
	
	glm::vec3 vel;
	glm::vec3 pos;
	glm::vec3 forward;
	float height;
	float width;
	
	float maxMovementSpeedHorizontal;
	bool movable;
	bool onFloor;
	
	std::string modelName;
	std::string userName;
	
	std::shared_ptr<icon7::Peer> peer;
	
public:
	void GetUpdateData(bitscpp::ByteWriter &writer)
	{
		writer.op(entityId);
		writer.op(lastUpdateTick);
		writer.op(vel);
		writer.op(pos);
		writer.op(forward);
	}
	
	template <typename S> S &__ByteStream_op(S &s)
	{
		s.op(entityId);
		s.op(lastUpdateTick);
		s.op(vel);
		s.op(pos);
		s.op(forward);
		s.op(height);
		s.op(width);
		s.op(maxMovementSpeedHorizontal);
		s.op(movable);
		s.op(modelName);
		s.op(userName);
		return s;
	}
};

inline Entity::Entity()
{
	realm = nullptr;

	vel = {0, 0, 0};
	pos = {0.5, 100, 0.5};
	forward = {0, 0, 1};
	height = 1.75;
	width = 0.6;
	onFloor = false;

	maxMovementSpeedHorizontal = 5;
	movable = false;
}

#include "PeerData.hpp"

inline void Entity::DisconnectPeer()
{
	if (peer) {
		// TODO: safe entity to database
		((PeerData *)(peer->userPointer))->realm = nullptr;
		((PeerData *)(peer->userPointer))->entityId = 0;
		peer = nullptr;
	}
}

inline Entity::~Entity()
{
	DisconnectPeer();
}
