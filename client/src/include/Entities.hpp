#pragma once

#include <cinttypes>

#include <unordered_map>
#include <string>

#include "../../../server/include/Timer.hpp"
#include "../../../server/include/Entity.hpp"

class Entities
{
public:
	Entities(class ClientConnection *clientConnection);
	
	void DeleteEntity(uint64_t entityId);
	void AddEntity(Entity *entity);

	// return fals if entity is not present
	bool UpdateEntity(uint64_t entityId, uint64_t lastUpdateTick, float *vel,
					  float *pos, float *forward);

	Entity *GetEntity(uint64_t entityId);
	
	void UpdateToCurrentTick(uint64_t entityId);
	void SetCurrentTick(uint64_t currentTick);
	
private:
	void UpdateToCurrentTick(Entity *entity);

public:
	Timer timer;

	class ClientConnection *const clientConnection;
	std::unordered_map<uint64_t, Entity> entities;
};
