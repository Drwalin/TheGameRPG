
#include <glm/geometric.hpp>

#include "Entity.hpp"
#include "Realm.hpp"

void Entity::Init(Realm *realm, uint64_t entityId)
{
	this->realm = realm;
	this->entityId = entityId;
	lastAuthoritativeState.timestamp = realm->GetCurrentTick();
	currentState = lastAuthoritativeState;
}

void Entity::Update(uint64_t currentTick)
{
	if (longState.movable) {
		EntityMovementState prev = lastAuthoritativeState;
		if (currentState.timestamp > prev.timestamp+1000) {
			prev = currentState;
		}
		auto &next = currentState;
		
		int64_t _dt = currentTick - prev.timestamp;
		if (_dt < 30) {
			return;
		}
		
		float dt = _dt * 0.001f;
		
		
		glm::vec3 acc = {0, -realm->gravity, 0};
		
		glm::vec3 vel = prev.vel + acc*dt;

		if (vel.y < -10) {
			vel.y = -10;
		}
		if (vel.y > 10) {
			vel.y = 10;
		}
		
		glm::vec3 movement = prev.vel*dt + acc*(dt*dt*0.5f);
		glm::vec3 pos = prev.pos + movement;
		
		float h = 0;
		if (realm->terrain.GetHeight(pos, &h)) {
			if (pos.y < h) { // here entity is touching ground
				pos.y = h;
				vel.y = 0;
			}
		}
		
		next.timestamp = currentTick;
		next.pos = pos;
		next.vel = vel;
		next.rot = prev.rot;
		
		DEBUG("Update player pos: %f %f %f", pos.x, pos.y, pos.z);
		
		
		
// 		uint64_t _dt = currentTick - lastUpdateTick;
// 		if (_dt < 30) {
// 			return;
// 		}
// 		lastUpdateTick = currentTick;
// 		float dt = _dt * 0.001f;
// 
// 		vel.y -= realm->gravity * dt;
// 
// 		if (vel.y < -100) {
// 			vel.y = -100;
// 		}
// 		if (vel.y > 100) {
// 			vel.y = 100;
// 		}
// 
// 		glm::vec3 oldPos = pos;
// 		glm::vec3 movement = vel * dt;
// 		glm::vec3 newPos = oldPos + movement;
// 
// 		pos = newPos;
// 
// 		float h = 0;
// 		if (realm->terrain.GetHeight(newPos, &h)) {
// 			if (newPos.y < h) {
// 				newPos.y = h;
// 				vel.y = 0;
// 			}
// 		}
// 
// 		glm::vec3 normal;
// 		float oldHeight;
// 		if (realm->terrain.GetHeightAndNormal(oldPos, &oldHeight, &normal)) {
// 			if (oldHeight + 1 > oldPos.y) {
// 				glm::vec3 v2 = vel;
// 				v2.y = 0;
// 				float v2l = glm::dot(v2, v2);
// 				if (v2l > 0.0001) {
// 					v2 /= v2l;
// 
// 					float d2 = glm::dot(v2, normal);
// 					if (d2 < -0.5) {
// 						pos = oldPos;
// 						pos.y = oldHeight;
// 					}
// 				}
// 			}
// 		}
	}
}

void Entity::SolvePlayerInput(uint64_t tick, glm::vec3 pos, glm::vec3 vel,
							  glm::vec3 rot)
{
	// TODO: verify if not cheating/speed hacking/itp.
	lastAuthoritativeState.timestamp = tick;
	lastAuthoritativeState.pos = pos;
	lastAuthoritativeState.vel = vel;
	lastAuthoritativeState.rot = rot;
	// TODO: maybe simulate?
}

void Entity::ConnectPeer(icon7::Peer *peer)
{
	PeerData *data = (PeerData *)(peer->userPointer);
	if (data->realm) {
		DEBUG("Error: Entity::ConnectPeer cannot be executed before "
			  "disconnecting from old realm.");
	}

	data->entityId = entityId;
	longState.userName = data->userName;
	longState.movable = true;
	this->peer = peer->shared_from_this();
	// TODO: load data from database
}

void Entity::SetModel(const std::string &modelName, float height, float width)
{
	this->longState.modelName = modelName;
	this->longState.height = height;
	this->longState.width = width;
	realm->UpdateModelOf(entityId, modelName, height, width);
}
