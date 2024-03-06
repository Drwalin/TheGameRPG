#include "../../../server/include/TerrainMap.hpp"
#include "../../../server/include/Entity.hpp"

#include <godot_cpp/variant/utility_functions.hpp>

#include <icon7/Flags.hpp>

#include <Rpc.hpp>

#include "../include/ClientConnection.hpp"
#include "../include/EntityPrefabScript.hpp"
#include "godot_cpp/classes/capsule_shape3d.hpp"
#include "icon7/Command.hpp"

void ClientConnection::RegisterMessages()
{
	rpc = &rpcHost->rpc;

	rpc->RegisterObjectMessage("UpdateTerrain", this,
							   &ClientConnection::UpdateTerrain,
							   &rpcHost->executionQueue);
	rpc->RegisterObjectMessage("SetRealms", this, &ClientConnection::SetRealms,
							   &rpcHost->executionQueue);
	rpc->RegisterObjectMessage("SpawnEntities", this,
							   &ClientConnection::SpawnEntities,
							   &rpcHost->executionQueue);
	rpc->RegisterObjectMessage("UpdateEntities", this,
							   &ClientConnection::UpdateEntities,
							   &rpcHost->executionQueue);
	rpc->RegisterObjectMessage("SetModel", this, &ClientConnection::SetModel,
							   &rpcHost->executionQueue);
	rpc->RegisterObjectMessage("DeleteEntities", this,
							   &ClientConnection::DeleteEntities,
							   &rpcHost->executionQueue);
	rpc->RegisterObjectMessage("SetPlayerEntityId", this,
							   &ClientConnection::SetPlayerEntityId,
							   &rpcHost->executionQueue);
	rpc->RegisterObjectMessage("SetGravity", this, &ClientConnection::SetGravity);
	rpc->RegisterObjectMessage("UpdateTimer", this, &ClientConnection::UpdateTimer);
	
}

void ClientConnection::SetRealms(std::vector<std::string> &realms)
{
	this->realms = realms;
	this->emit_signal("_ReceivedRealmsList");
}

void ClientConnection::SpawnEntities(icon7::ByteReader *reader)
{
	uint64_t entityId;
	EntityMovementState movementState;
	EntityLongState longState;
	while (reader->has_any_more() && reader->is_valid()) {
		reader->op(entityId);
		reader->op(movementState);
		reader->op(longState);
		if (reader->is_valid()) {
			entities.AddEntity(entityId, movementState, longState);
		}
	}
}

void ClientConnection::UpdateEntities(icon7::ByteReader *reader)
{
	std::vector<uint64_t> notPresentEntities;
	while (reader->has_any_more() && reader->is_valid()) {
		uint64_t entityId;
		EntityMovementState movementState;
		reader->op(entityId);
		reader->op(movementState);
		if (reader->is_valid()) {
			if (playerEntityId != entityId) {
				if (entities.UpdateEntity(entityId, movementState) == false) {
					notPresentEntities.emplace_back(entityId);
				}
			}
		}
	}
	std::vector<uint8_t> buffer;
	{
		bitscpp::ByteWriter writer(buffer);
		writer.op(ServerRemoteFunctions::GetEntitiesData);
		writer.op(notPresentEntities.data(), notPresentEntities.size());
	}
	peer->peer->Send(std::move(buffer), icon7::FLAG_RELIABLE|icon7::FLAGS_CALL_NO_FEEDBACK);
}

void ClientConnection::SetModel(uint64_t entityId, std::string_view modelName,
								float height, float width)
{
	auto entity = entities.GetEntity(entityId);
	bool valid = false;
	if (entity) {
		if (entity->godotNode) {
			if (entity->godotNode->collisionShape) {
				valid = true;
			}
		}
	}
	if (valid == false) {
		icon7::commands::ExecuteFunction com;
		com.function = std::bind(&ClientConnection::SetModel, this, entityId, (std::string)modelName, height, width);
		rpcHost->executionQueue.EnqueueCommand(std::move(com));
	} else {
		if (entity) {
			entity->longState.height = height;
			entity->longState.width = width;
			if (entity->godotNode) {
				if (entity->godotNode->capsuleShape == nullptr) {
					auto capsule = new godot::CapsuleShape3D();
					entity->godotNode->capsuleShape = capsule;
					entity->godotNode->collisionShape->set_shape(capsule);
				}
				entity->godotNode->capsuleShape->set_height(height);
				entity->godotNode->capsuleShape->set_radius(width*0.5);
				entity->godotNode->collisionShape->set_position({0, height*0.5f, 0});
				// TODO: set model by name
			}
		}
	}
}

void ClientConnection::DeleteEntities(icon7::ByteReader *reader)
{
	while (reader->has_any_more() && reader->is_valid()) {
		uint64_t entityId;
		reader->op(entityId);
		if (reader->is_valid()) {
			entities.DeleteEntity(entityId);
			// TODO: erase entity
		}
	}
}

void ClientConnection::SetPlayerEntityId(uint64_t playerEntityId)
{
	this->playerEntityId = playerEntityId;
}

void ClientConnection::SetGravity(float gravity)
{
	this->gravity = {0, -gravity, 0};
}

void ClientConnection::UpdateTimer(uint64_t currentTick)
{
	entities.SetCurrentTick(currentTick);
}
