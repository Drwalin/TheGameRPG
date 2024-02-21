#include "../../../server/include/TerrainMap.hpp"
#include "../../../server/include/Entity.hpp"

#include "../include/ClientConnection.hpp"
#include "Rpc.hpp"
#include "icon7/Flags.hpp"

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
}

void ClientConnection::SetRealms(std::vector<std::string> &realms)
{
	this->realms = realms;
	this->emit_signal("_ReceivedRealmsList");
}

void ClientConnection::SpawnEntities(icon7::ByteReader *reader)
{
	Entity entity;
	while (reader->has_any_more() && reader->is_valid()) {
		reader->op(entity);
		if (reader->is_valid()) {
			entities.AddEntity(&entity);
		}
	}
}

void ClientConnection::UpdateEntities(icon7::ByteReader *reader)
{
	std::vector<uint64_t> notPresentEntities;
	while (reader->has_any_more() && reader->is_valid()) {
		uint64_t entityId, lastUpdateTick;
		float vel[3], pos[3], forward[3];
		reader->op(entityId);
		reader->op(lastUpdateTick);
		reader->op(vel, 3);
		reader->op(pos, 3);
		reader->op(forward, 3);
		if (reader->is_valid()) {
			if (entities.UpdateEntity(entityId, lastUpdateTick, vel, pos, forward) == false) {
				notPresentEntities.emplace_back(entityId);
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
	// TODO: set model
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
