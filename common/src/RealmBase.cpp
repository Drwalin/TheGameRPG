
#include "../include/RealmBase.hpp"

RealmBase::RealmBase()
{
	collisionWorld = CollisionWorld::Allocate();
}

RealmBase::~RealmBase()
{
	CollisionWorld::Free(collisionWorld);
	collisionWorld = nullptr;
}

uint64_t RealmBase::AddEntity()
{
	uint64_t entityId = _InternalAddEntity();
	EntityBase *entity = GetEntity(entityId);
	collisionWorld->AddEntity(entityId, entity->longState.shape, entity->currentState.pos);
	return entityId;
}

void RealmBase::DestroyEntity(uint64_t entityId)
{
	_InternalDestroyEntity(entityId);
	collisionWorld->DeleteEntity(entityId);
}

void RealmBase::SetEntityModel(uint64_t entityId, const std::string &modelName,
				   float width, float height)
{
	EntityBase *entity = GetEntity(entityId);
	if (entity) {
		entity->SetModel(modelName, width, height);
	}
}
