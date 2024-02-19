
#include <glm/geometric.hpp>

#include "Entity.hpp"
#include "Realm.hpp"

Entity::Entity()
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

void Entity::Init(Realm *realm, uint64_t entityId)
{
	this->realm = realm;
	this->entityId = entityId;
	lastUpdateTick = realm->GetCurrentTick();
}

Entity::~Entity()
{
	DisconnectPeer();
}

void Entity::Update(uint64_t currentTick)
{
	if (movable) {
		onFloor = false;
		uint64_t _dt = currentTick - lastUpdateTick;
		if (_dt < 30) {
			return;
		}
		lastUpdateTick = currentTick;
		float dt = _dt * 0.001f;

		vel.y -= realm->gravity * dt;

		if (vel.y < -100) {
			vel.y = -100;
		}
		if (vel.y > 100) {
			vel.y = 100;
		}

		glm::vec3 oldPos = pos;
		glm::vec3 movement = vel * dt;
		glm::vec3 newPos = oldPos + movement;

		pos = newPos;

		float h = 0;
		if (realm->terrain.GetHeight(newPos, &h)) {
			if (newPos.y < h) {
				newPos.y = h;
				vel.y = 0;
				onFloor = true;
			}
		}

		glm::vec3 normal;
		float oldHeight;
		if (realm->terrain.GetHeightAndNormal(oldPos, &oldHeight, &normal)) {
			if (oldHeight + 1 > oldPos.y) {
				glm::vec3 v2 = vel;
				v2.y = 0;
				float v2l = glm::dot(v2, v2);
				if (v2l > 0.0001) {
					v2 /= v2l;

					float d2 = glm::dot(v2, normal);
					if (d2 < -0.5) {
						pos = oldPos;
						pos.y = oldHeight;
						onFloor = true;
					}
				}
			}
		}
	}
}

void Entity::SolvePlayerInput(uint64_t tick, glm::vec3 pos, glm::vec3 vel,
							  glm::vec3 forward)
{
	// TODO: verify if not cheating/speed hacking/itp.
	lastUpdateTick = tick;
	this->pos = pos;
	this->vel = vel;
	this->forward = forward;
}

void Entity::ConnectPeer(icon7::Peer *peer)
{
	DisconnectPeer();
	((PeerData *)(peer->userPointer))->entityId = entityId;
	userName = ((PeerData *)(peer->userPointer))->userName;
	this->peer = peer->shared_from_this();
	// TODO: load data from database
}

void Entity::DisconnectPeer()
{
	if (peer) {
		// TODO: safe entity to database
		((PeerData *)(peer->userPointer))->realm = nullptr;
		((PeerData *)(peer->userPointer))->entityId = 0;
		peer = nullptr;
	}
}

void Entity::SetModel(const std::string &modelName, float height, float width)
{
	this->modelName = modelName;
	this->height = height;
	this->width = width;
	realm->UpdateModelOf(entityId, modelName, height, width);
}
