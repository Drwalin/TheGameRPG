#pragma once

#include <cstdio>
#include <cinttypes>

#include <string>

#include <glm/glm.hpp>

#include <icon7/Peer.hpp>

#include "GlmSerialization.hpp"

struct EntityMovementState
{
	uint64_t timestamp = 0;
	glm::vec3 pos = {10,100,10};
	glm::vec3 vel = {0,0,0};
	glm::vec3 rot = {0,0,0};
	
	template <typename S> S &__ByteStream_op(S &s)
	{
		s.op(timestamp);
		s.op(pos);
		s.op(vel);
		s.op(rot);
		return s;
	}
};

struct EntityLongState
{
	float height = 1.75;
	float width = 0.6;
	
	float maxMovementSpeedHorizontal = 5;
	bool movable = false;
	
	std::string modelName;
	std::string userName;
	
	template <typename S> S &__ByteStream_op(S &s)
	{
		s.op(height);
		s.op(width);
		s.op(maxMovementSpeedHorizontal);
		s.op(movable);
		s.op(modelName);
		s.op(userName);
		return s;
	}
};


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
						  glm::vec3 rot);
	
	void SetModel(const std::string &modelName, float height, float width);
	
public:
	Realm *realm;
	
	uint64_t entityId;
	
	EntityMovementState lastAuthoritativeState;
	EntityMovementState currentState;
	
	EntityLongState longState;
	
	std::shared_ptr<icon7::Peer> peer;
	
	static void SimulateMovement(Entity *entity);
	
public:
	void GetUpdateData(bitscpp::ByteWriter &writer)
	{
		writer.op(entityId);
		if (currentState.timestamp > lastAuthoritativeState.timestamp+500) {
			writer.op(currentState);
		} else {
			writer.op(lastAuthoritativeState);
		}
	}
	
	template <typename S> S &__ByteStream_op(S &s)
	{
		s.op(entityId);
		if constexpr (std::is_same_v<S, bitscpp::ByteWriter>) {
			if (lastAuthoritativeState.timestamp > currentState.timestamp) {
				s.op(lastAuthoritativeState);
			} else {
				s.op(currentState);
			}
		} else {
			s.op(lastAuthoritativeState);
			currentState = lastAuthoritativeState;
		}
		s.op(longState);
		return s;
	}
};

inline Entity::Entity()
{
	realm = nullptr;
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
