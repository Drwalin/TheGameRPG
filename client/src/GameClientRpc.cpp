#include <icon7/PeerUStcp.hpp>
#include <icon7/HostUStcp.hpp>
#include <icon7/Command.hpp>
#include <icon7/Flags.hpp>

#include "../include/ServerRpcProxy.hpp"

#include "../include/GameClient.hpp"
#include "ClientRpcFunctionNames.hpp"

void GameClient::BindRpc()
{
	rpc.RegisterObjectMessage(ClientRpcFunctionNames::JoinRealm, this,
							  &GameClient::JoinRealm, &executionQueue);
	rpc.RegisterObjectMessage(ClientRpcFunctionNames::SpawnEntities, this,
							  &GameClient::SpawnEntities, &executionQueue);
	rpc.RegisterObjectMessage(ClientRpcFunctionNames::UpdateEntities, this,
							  &GameClient::UpdateEntities, &executionQueue);
	rpc.RegisterObjectMessage(ClientRpcFunctionNames::SetModel, this,
							  &GameClient::SetModel, &executionQueue);
	rpc.RegisterObjectMessage(ClientRpcFunctionNames::DeleteEntities, this,
							  &GameClient::DeleteEntities, &executionQueue);
	rpc.RegisterObjectMessage(ClientRpcFunctionNames::SetPlayerEntityId, this,
							  &GameClient::SetPlayerEntityId, &executionQueue);
	rpc.RegisterObjectMessage(ClientRpcFunctionNames::SetGravity, this,
							  &GameClient::SetGravity, &executionQueue);
	rpc.RegisterObjectMessage(ClientRpcFunctionNames::LoginFailed, this,
							  &GameClient::LoginFailed, &executionQueue);
	rpc.RegisterObjectMessage(ClientRpcFunctionNames::LoginSuccessfull, this,
							  &GameClient::LoginSuccessfull, &executionQueue);
	rpc.RegisterObjectMessage(ClientRpcFunctionNames::Pong, this,
							  &GameClient::Pong, &executionQueue);
}

void GameClient::JoinRealm(const std::string &realmName)
{
	realm.Reinit(realmName);
}

void GameClient::SpawnEntities(icon7::ByteReader *reader)
{
	uint64_t serverId;
	EntityLastAuthoritativeMovementState state;
	EntityName name;
	EntityModelName model;
	EntityShape shape;
	EntityMovementParameters movementParams;
	while (reader->get_remaining_bytes() > 8) {
		reader->op(serverId);
		reader->op(state);
		reader->op(name);
		reader->op(model);
		reader->op(shape);
		reader->op(movementParams);

		if (reader->is_valid()) {
			SpawnEntity(serverId, state, name, model, shape, movementParams);
		}
	}
}

void GameClient::UpdateEntities(icon7::ByteReader *reader)
{
	uint64_t serverId;
	EntityLastAuthoritativeMovementState state;
	while (reader->get_remaining_bytes() > 8) {
		reader->op(serverId);
		reader->op(state);

		if (reader->is_valid()) {
			UpdateEntity(serverId, state);
		}
	}
}

void GameClient::SetModel(uint64_t serverId, EntityModelName model,
						  EntityShape shape)
{
	uint64_t localId = 0;
	auto it = mapServerEntityIdToLocalEntityId.find(serverId);
	if (it == mapServerEntityIdToLocalEntityId.end()) {
		RequestSpawnOf(serverId);
		return;
	} else {
		localId = it->second;
	}

	realm.SetComponent(localId, shape);
	realm.SetComponent(localId, model);

	OnEntityShape(localId, shape);
	OnEntityModel(localId, model);
}

void GameClient::DeleteEntities(icon7::ByteReader *reader)
{
	uint64_t serverId;
	while (reader->get_remaining_bytes() >= 8) {
		reader->op(serverId);

		if (reader->is_valid()) {
			RemoveEntity(serverId);
		}
	}
}

void GameClient::SetPlayerEntityId(uint64_t serverId)
{
	serverPlayerEntityId = serverId;
	auto it = mapServerEntityIdToLocalEntityId.find(serverId);
	if (it == mapServerEntityIdToLocalEntityId.end()) {
		RequestSpawnOf(serverId);
		return;
	} else {
		localPlayerEntityId = it->second;
		OnSetPlayerId(localPlayerEntityId);
	}
}

void GameClient::SetGravity(float gravity) { realm.gravity = gravity; }

void GameClient::LoginFailed()
{
	// TODO: ???
}

void GameClient::LoginSuccessfull()
{
	// TODO: ???
}

void GameClient::Pong(int64_t localTick, int64_t remoteTick)
{
	int64_t currentTick = pingTimer.CalcCurrentTick();
	pingMs = currentTick - localTick;
	DEBUG("ping = %ld ms", pingMs);
	if (remoteTick != 0) {
		// TODO: consider not adding latency?
		int64_t newCurrentTick = remoteTick + pingMs / 2;
		if (newCurrentTick < realm.timer.currentTick) {
			// TODO: time error??
			DEBUG("Ping/Pong received time invalid");
		} else {
			realm.timer.Start(newCurrentTick);
			DEBUG("current tick = %lu", newCurrentTick);
		}
	}
}

void GameClient::SpawnEntity(uint64_t serverId,
							 const EntityLastAuthoritativeMovementState state,
							 const EntityName name, const EntityModelName model,
							 const EntityShape shape,
							 const EntityMovementParameters movementParams)
{
	uint64_t localId = 0;
	auto it = mapServerEntityIdToLocalEntityId.find(serverId);
	if (it == mapServerEntityIdToLocalEntityId.end()) {
		localId = realm.NewEntity();
		mapServerEntityIdToLocalEntityId[serverId] = localId;
		mapLocalEntityIdToServerEntityId[localId] = serverId;
	} else {
		localId = it->second;
	}

	realm.SetComponent(localId, name);
	realm.SetComponent(localId, state);
	realm.SetComponent(localId, model);
	realm.SetComponent(localId, shape);
	realm.SetComponent(localId, movementParams);

	OnEntityAdd(localId);

	if (serverId == serverPlayerEntityId) {
		localPlayerEntityId = localId;
		OnSetPlayerId(localPlayerEntityId);
	}
}

void GameClient::UpdateEntity(uint64_t serverId,
							  const EntityLastAuthoritativeMovementState state)
{
	uint64_t localId = 0;
	auto it = mapServerEntityIdToLocalEntityId.find(serverId);
	if (it == mapServerEntityIdToLocalEntityId.end()) {
		RequestSpawnOf(serverId);
		return;
	} else {
		localId = it->second;
	}

	if (localId != localPlayerEntityId) {
		realm.SetComponent(localId, state);
	}
}

void GameClient::RemoveEntity(uint64_t serverId)
{
	uint64_t localId = 0;
	auto it = mapServerEntityIdToLocalEntityId.find(serverId);
	if (it == mapServerEntityIdToLocalEntityId.end()) {
		return;
	} else {
		localId = it->second;
	}

	OnEntityRemove(localId);

	if (serverId == serverPlayerEntityId) {
		OnPlayerIdUnset();
		serverPlayerEntityId = 0;
		localPlayerEntityId = 0;
	}

	realm.RemoveEntity(localId);

	mapServerEntityIdToLocalEntityId.erase(serverId);
	mapLocalEntityIdToServerEntityId.erase(localId);
}

void GameClient::Login(const std::string &username, const std::string &password)
{
	ServerRpcProxy::Login(this, username, password);
}

void GameClient::RequestSpawnOf(uint64_t serverId)
{
	// TODO: better (caching) implementation
	ServerRpcProxy::GetEntitiesData(this, {serverId});
}
