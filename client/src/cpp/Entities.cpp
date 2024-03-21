#include "godot_cpp/variant/utility_functions.hpp"
#include "godot_cpp/classes/scene_tree.hpp"

#include "../include/Entities.hpp"
#include "../include/ClientConnection.hpp"
#include "../include/EntityPrefabScript.hpp"

Entities::Entities(class ClientConnection *clientConnection)
	: clientConnection(clientConnection)
{
}

void Entities::DeleteEntity(uint64_t entityId) { entities.erase(entityId); }

void Entities::AddEntity(uint64_t entityId, EntityMovementState &movementState,
			EntityLongState &longState)
{
	ClientEntity *e2 = &(entities[entityId]);
	e2->entityId = entityId;
	e2->movementState = movementState;
	e2->longState = longState;
	// TODO: maybe? do Update to current tick here?
	
	if (e2->godotNode == nullptr) {
// 		DEBUG("START");
		if (prefab==nullptr) {
			auto x =
				godot::ResourceLoader::get_singleton()->load("res://prefabs/MovingEntity.tscn", "PackedScene");
			DEBUG(" x = %p", x.ptr());
			prefab = x;
		}
// 		DEBUG("A");
		e2->godotNode = (EntityPrefabScript *)(prefab->instantiate());
// 		DEBUG("B, godotNode = %lu", e2->godotNode);
		e2->entityId = entityId;
// 		DEBUG("C");
		auto *entitiesNode = clientConnection->get_node<godot::Node>("/root/SceneRoot/Entities");
// 		DEBUG("D");
		godot::UtilityFunctions::print("Godot node: ", e2->godotNode, " ; ptr: ", (uint64_t)(void*)(e2->godotNode));
		e2->godotNode->Init(entityId);
		entitiesNode->add_child(e2->godotNode);
// 		DEBUG("END");
		
		clientConnection->SetModel(entityId, e2->longState.modelName, e2->longState.height, e2->longState.width);
	}
}

bool Entities::UpdateEntity(uint64_t entityId, EntityMovementState &movementState)
{
	ClientEntity *entity = GetEntity(entityId);
	if (entity == nullptr) {
		return false;
	}
	entity->movementState = movementState;
// 	DEBUG("Updated entity at time %lu with pos: {%4.8f, %4.8f, %4.8f}", movementState.timestamp, movementState.pos.x, movementState.pos.y, movementState.pos.z);
// 	godot::UtilityFunctions::print("Updated entity with pos: ", movementState.pos.x, " ", movementState.pos.y, " ", movementState.pos.z, " ; at time: ", movementState.timestamp);
	// TODO: maybe? do Update to current tick here?
	return true;
}

ClientEntity *Entities::GetEntity(uint64_t entityId)
{
	auto it = entities.find(entityId);
	if (it != entities.end()) {
		return &(it->second);
	}
	return nullptr;
}

void Entities::Update()
{
	// TODO: update, what to update
	uint64_t dt;
	timer.Update(10, &dt, nullptr);
}

void Entities::SetCurrentTick(uint64_t currentTick)
{
	timer.Start(currentTick);
}
