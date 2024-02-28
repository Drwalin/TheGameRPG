#pragma once

#include <cinttypes>

#include <unordered_map>
#include <string>

#include <godot_cpp/core/defs.hpp>
#include <godot_cpp/godot.hpp>
#include <godot_cpp/classes/scene_tree.hpp>
#include "godot_cpp/classes/resource_preloader.hpp"
#include "godot_cpp/classes/resource_loader.hpp"
#include <godot_cpp/classes/node2d.hpp>
#include <godot_cpp/classes/packed_scene.hpp>

#include "../../../server/include/Timer.hpp"
#include "../../../server/include/Entity.hpp"

class ClientEntity
{
public:
	
	uint64_t entityId = 0;
	EntityMovementState movementState;
	EntityLongState longState;
	class EntityPrefabScript *godotNode = nullptr;
};

class Entities
{
public:
	Entities(class ClientConnection *clientConnection);
	
	void DeleteEntity(uint64_t entityId);
	void AddEntity(uint64_t entityId, EntityMovementState &movementState,
			EntityLongState &longState);

	// return fals if entity is not present
	bool UpdateEntity(uint64_t entityId, EntityMovementState &movementState);

	ClientEntity *GetEntity(uint64_t entityId);
	
	void SetCurrentTick(uint64_t currentTick);
	
	void Update();
	
public:
	Timer timer;

	class ClientConnection *const clientConnection;
	std::unordered_map<uint64_t, ClientEntity> entities;
	
	godot::Ref<godot::PackedScene> prefab;
};
