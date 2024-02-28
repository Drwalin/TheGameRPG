#include "../include/Entities.hpp"

Entities::Entities(class ClientConnection *clientConnection)
	: clientConnection(clientConnection)
{
}

void Entities::DeleteEntity(uint64_t entityId) { entities.erase(entityId); }

void Entities::AddEntity(Entity *entity)
{
	Entity *e2 = &(entities[entity->entityId]);
	*e2 = *entity;
	UpdateToCurrentTick(e2);
}

bool Entities::UpdateEntity(uint64_t entityId, uint64_t lastUpdateTick,
							float *vel, float *pos, float *forward)
{
	Entity *entity = GetEntity(entityId);
	if (entity == nullptr) {
		return false;
	}
	entity->lastUpdateTick = lastUpdateTick;
	memcpy(&entity->vel, vel, 3 * sizeof(float));
	memcpy(&entity->vel, vel, 3 * sizeof(float));
	memcpy(&entity->vel, vel, 3 * sizeof(float));
	UpdateToCurrentTick(entity);
	return true;
}

Entity *Entities::GetEntity(uint64_t entityId)
{
	auto it = entities.find(entityId);
	if (it != entities.end()) {
		return &(it->second);
	}
	return nullptr;
}

void Entities::SetCurrentTick(uint64_t currentTick)
{
	timer.Start(currentTick);
}

void Entities::UpdateToCurrentTick(uint64_t entityId)
{
	UpdateToCurrentTick(GetEntity(entityId));
}

void Entities::UpdateToCurrentTick(Entity *entity)
{
	uint64_t currentTick = timer.CalcCurrentTick();
	if (entity->lastUpdateTick + 5 < currentTick) {
		// TODO: create client side update
// 		entity->Update(currentTick);
	}
}
