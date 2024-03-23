#pragma once

#include <cinttypes>

#include <unordered_map>

template<typename T>
class EntityStorage
{
public:
	uint64_t GenerateNewId()
	{
		++lastEntityIdUsed;
		while (lastEntityIdUsed == 0 ||
			   entities.find(lastEntityIdUsed) != entities.end()) {
			++lastEntityIdUsed;
		}
		return lastEntityIdUsed;
	}

	template<typename Arg1, typename... Args>
	uint64_t Add(Arg1 arg1, Args... args)
	{
		uint64_t entityId = GenerateNewId();
		T &entity = entities[entityId];
		entity.Init(arg1, entityId, args...);
		return entityId;
	}

	T *Get(uint64_t entityId)
	{
		auto it = entities.find(entityId);
		if (it != entities.end()) {
			return &it->second;
		}
		return nullptr;
	}

	uint64_t Destroy(uint64_t entityId)
	{
		T &entity = entities[entityId];
		entities.erase(entityId);
	}
	
	template<typename Fun, typename... Args>
	void ForEach(Fun &&fun, Args... args)
	{
		for (auto it : entities) {
			fun(it->second, args...);
		}
	}
	
public:
	std::unordered_map<uint64_t, T> entities;
	uint64_t lastEntityIdUsed;
};
