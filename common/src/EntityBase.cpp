#include "../include/RealmBase.hpp"
#include "../include/CollisionWorld.hpp"

#include "../include/EntityBase.hpp"

EntityBase::EntityBase() : longState() {}

EntityBase::~EntityBase() {
	longState.~EntityLongState();
}

void EntityBase::Init(RealmBase *realm, uint64_t entityId)
{
	this->realm = realm;
	this->entityId = entityId;
	lastAuthoritativeState.timestamp = realm->timer.currentTick;
	currentState = lastAuthoritativeState;
}

void EntityBase::Update(uint64_t currentTick)
{
	EntityMovementState prev = lastAuthoritativeState;
	if (currentState.timestamp > prev.timestamp+realm->ticksBeforeIgnoringInputMovement) {
		prev = currentState;
	}
	auto &next = currentState;
	bool onGround = next.onGround;
	
	int64_t _dt = currentTick - prev.timestamp;
	if (_dt < realm->minDeltaTicks) {
		return;
	}
	
	float dt = _dt * 0.001f;
	
	if (onGround == true && prev.vel.y < 0.01) {
		if (lastAuthoritativeState.timestamp+realm->ticksBeforeIgnoringInputMovement < next.timestamp) {
			if (lastAuthoritativeState.timestamp+realm->ticksBeforeIgnoringInputMovement < currentTick ) {
				return;
			}
		}
		
		glm::vec3 vel = prev.vel;
		vel.y = 0;
		if (fabs(vel.x)+fabs(vel.z) < 0.001) {
			next = prev;
			next.timestamp = currentTick;
			return;
		}
		
		// step stepping algorithm:
		
		// step 1: up
		glm::vec3 oldPos = prev.pos;
		glm::vec3 newPos = oldPos + glm::vec3(0, longState.stepHeight, 0);
		glm::vec3 pos;
		realm->collisionWorld->TestCollisionMovement(longState.shape, oldPos, newPos, &pos, nullptr);
		newPos = pos;
		
		// step 2: vertical
		glm::vec3 movement = vel * dt;
		float heightDiff = newPos.y - oldPos.y;
		oldPos = newPos;
		newPos = oldPos + movement;
		realm->collisionWorld->TestCollisionMovement(longState.shape, oldPos, newPos, &pos, nullptr);
		newPos = pos;
		
		// step 3: down
		oldPos = newPos;
		newPos = oldPos - glm::vec3(0, heightDiff+longState.stepHeight, 0);
		bool isOnGround = false;
		if (realm->collisionWorld->TestCollisionMovement(longState.shape, oldPos, newPos, &pos, &isOnGround)) {
			onGround = isOnGround;
		}
		
		vel = (pos - prev.pos)/dt;
		if (onGround) {
			vel.y = 0;
		}
		
		next.timestamp = currentTick;
		next.pos = pos;
		next.vel = vel;
		next.rot = prev.rot;
		next.onGround = onGround;
	} else {
		glm::vec3 acc = {0, -realm->gravity, 0};
		
		glm::vec3 vel = prev.vel + acc*dt;

		if (vel.y < -50) {
			vel.y = -50;
		}
		if (vel.y > 50) {
			vel.y = 50;
		}
		
		glm::vec3 oldPos = prev.pos;
		glm::vec3 movement = prev.vel*dt + acc*(dt*dt*0.5f);
		glm::vec3 newPos = oldPos + movement;
	
		// test collision here:
		glm::vec3 pos;
		bool isOnGround = false;
		if (realm->collisionWorld->TestCollisionMovement(longState.shape, oldPos, newPos, &pos, &isOnGround)) {
			if (vel.y <= 0) {
				onGround = isOnGround;
			}
		}
		
		vel = (pos-oldPos) / dt;
		
		next.timestamp = currentTick;
		next.pos = pos;
		next.vel = vel;
		next.rot = prev.rot;
		next.onGround = onGround;
	}
	
	realm->collisionWorld->UpdateEntityBvh(entityId, longState.shape, currentState.pos);
	
// 	DEBUG("Update player pos: %f %f %f", pos.x, pos.y, pos.z);
}

void EntityBase::SolvePlayerInput(const EntityMovementState &state)
{
	// TODO: verify if not cheating/speed hacking/itp.
	lastAuthoritativeState = state;
	currentState = state;
	// TODO: maybe simulate?
	realm->collisionWorld->UpdateEntityBvh(entityId, longState.shape, currentState.pos);
}

void EntityBase::SetModel(const std::string &modelName, float height, float width)
{
	this->longState.modelName = modelName;
	this->longState.shape.height = height;
	this->longState.shape.width = width;
	realm->collisionWorld->UpdateEntityBvh(entityId, longState.shape, currentState.pos);
}
