#include <icon7/ByteReader.hpp>
#include <icon7/Debug.hpp>

#include "../include/ServerRpcProxy.hpp"
#include "../include/RegistryComponent.hpp"
#include "../include/EntityComponentsClient.hpp"

#include "../include/GameClient.hpp"

void GameClient::JoinRealm(const std::string &realmName, int64_t currentTick,
						   uint64_t playerEntityId)
{
	if (serverPlayerEntityId || localPlayerEntityId) {
		OnPlayerIdUnset();
		serverPlayerEntityId = 0;
		localPlayerEntityId = 0;
	}
	realm->timer.Start(currentTick);
	realm->Reinit(realmName);
	SetPlayerEntityId(playerEntityId);
}

void GameClient::SpawnEntities(icon7::ByteReader *reader)
{
	uint64_t serverId;
	ComponentLastAuthoritativeMovementState state;
	ComponentName name;
	ComponentModelName model;
	ComponentShape shape;
	ComponentMovementParameters movementParams;
	while (reader->get_remaining_bytes() > 8) {
		reader->op(serverId);
		reader->op(state);
		reader->op(name);
		reader->op(model);
		reader->op(shape);
		reader->op(movementParams);

		if (reader->is_valid()) {
			SpawnEntity(serverId, state, name, model, shape, movementParams);
		} else {
			LOG_WARN("Reader became not valid.");
		}
	}
	ServerRpcProxy::Ping(this, true);
}

void GameClient::SpawnStaticEntities(icon7::ByteReader *reader)
{
	uint64_t serverId;
	ComponentStaticTransform transform;
	ComponentModelName model;
	ComponentStaticCollisionShapeName shape;
	while (reader->get_remaining_bytes() > 8) {
		reader->op(serverId);
		reader->op(transform);
		reader->op(model);
		reader->op(shape);

		if (reader->is_valid()) {
			auto it = mapServerEntityIdToLocalEntityId.find(serverId);
			if (it == mapServerEntityIdToLocalEntityId.end()) {
				SpawnStaticEntity(serverId, transform, model, shape);
			} else {
				realm->SetComponent(it->second, transform);
			}
		}
	}
}

void GameClient::UpdateEntities(icon7::ByteReader *reader)
{
	uint64_t serverId;
	ComponentLastAuthoritativeMovementState state;
	while (reader->get_remaining_bytes() > 8) {
		reader->op(serverId);
		reader->op(state);

		if (reader->is_valid()) {
			auto s = state.oldState;
			UpdateEntity(serverId, state);
		}
	}
}

void GameClient::SetModel(uint64_t serverId, ComponentModelName model,
						  ComponentShape shape)
{
	uint64_t localId = 0;
	auto it = mapServerEntityIdToLocalEntityId.find(serverId);
	if (it == mapServerEntityIdToLocalEntityId.end()) {
		RequestSpawnOf(serverId);
		return;
	} else {
		localId = it->second;
	}

	realm->SetComponent(localId, shape);
	realm->SetComponent(localId, model);
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
		localPlayerEntityId = 0;
		RequestSpawnOf(serverId);
		LOG_DEBUG("Set player entity id: [%lu]", serverId);
	} else {
		localPlayerEntityId = it->second;
		OnSetPlayerId(localPlayerEntityId);
		LOG_DEBUG("Set player entity id: [%lu > %lu]", serverId,
				  localPlayerEntityId);
	}
}

void GameClient::SetGravity(float gravity) { realm->gravity = gravity; }

void GameClient::Pong(int64_t localTick, int64_t remoteTick)
{
	realm->timer.Update();
	int64_t currentTick = realm->timer.currentTick;
	pingMs = pingTimer.CalcCurrentTick() - localTick;
	if (remoteTick != 0) {
		// TODO: consider not adding latency?
		int64_t newCurrentTick = remoteTick; // + pingMs / 2;
		if (abs(newCurrentTick - currentTick) > 500) {
			realm->timer.Start(newCurrentTick);
		} else if (newCurrentTick < currentTick) {
			// TODO: time error??
			if (newCurrentTick + 5 < currentTick) {
				newCurrentTick = currentTick - 5;
				if (newCurrentTick + 15 < currentTick) {
					ServerRpcProxy::Ping(this, true);
				}
			}
			realm->timer.Start(newCurrentTick);
		} else {
			if (newCurrentTick > currentTick + 25) {
				newCurrentTick = currentTick + 25;
				ServerRpcProxy::Ping(this, true);
			}
			realm->timer.Start(newCurrentTick);
		}
	}
}

void GameClient::SpawnEntity(
	uint64_t serverId, const ComponentLastAuthoritativeMovementState &state,
	const ComponentName &name, const ComponentModelName &model,
	const ComponentShape &shape,
	const ComponentMovementParameters &movementParams)
{
	uint64_t localId = 0;
	auto it = mapServerEntityIdToLocalEntityId.find(serverId);
	if (it == mapServerEntityIdToLocalEntityId.end()) {
		localId = realm->NewEntity();
		mapServerEntityIdToLocalEntityId[serverId] = localId;
		mapLocalEntityIdToServerEntityId[localId] = serverId;
	} else {
		localId = it->second;
	}

	realm->SetComponent(localId, shape);
	realm->SetComponent(localId, state);
	realm->SetComponent(localId, movementParams);
	realm->SetComponent(localId, name);
	realm->AssureComponent<ComponentEventsQueue>(localId);
	realm->SetComponent(localId, state.oldState);
	realm->SetComponent(localId, model);

	if (serverId == serverPlayerEntityId) {
		localPlayerEntityId = localId;
		OnSetPlayerId(localPlayerEntityId);
		// LOG_DEBUG("Spawn   player: [%lu>%lu]", serverId, localId);
	} else {
		// LOG_DEBUG("Spawn   entity: [%lu>%lu]     %lu != %lu", serverId,
		// 			 localId, serverId, serverPlayerEntityId);
	}
}

void GameClient::SpawnStaticEntity(uint64_t serverId,
								   ComponentStaticTransform transform,
								   ComponentModelName model,
								   ComponentStaticCollisionShapeName shape)
{
	uint64_t localId = realm->CreateStaticEntity(transform, model, shape);
	mapServerEntityIdToLocalEntityId[serverId] = localId;
	mapLocalEntityIdToServerEntityId[localId] = serverId;
}

void GameClient::UpdateEntity(
	uint64_t serverId, const ComponentLastAuthoritativeMovementState state)
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
		realm->SetComponent<ComponentLastAuthoritativeStateUpdateTime>(
			localId, ComponentLastAuthoritativeStateUpdateTime{
						 std::chrono::steady_clock::now(),
						 realm->timer.currentTick, state.oldState.timestamp});
		realm->AddNewAuthoritativeMovementState(localId, serverId, state);
		realm->UpdateEntityCurrentState(localId, serverId);
	} else {
		// TODO: implement server authority correction of client-side player
		//       entity movement
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

	if (serverId == serverPlayerEntityId) {
		OnPlayerIdUnset();
		serverPlayerEntityId = 0;
		localPlayerEntityId = 0;
	}

	realm->RemoveEntity(localId);

	mapServerEntityIdToLocalEntityId.erase(serverId);
	mapLocalEntityIdToServerEntityId.erase(localId);

	LOG_DEBUG("Despawn entity: [%lu>%lu]", serverId, localId);
}

void GameClient::Login(const std::string &username)
{
	ServerRpcProxy::Login(this, username);
}

void GameClient::RequestSpawnOf(uint64_t serverId)
{
	// TODO: better (caching) implementation
	ServerRpcProxy::GetEntitiesData(this, {serverId});
}

void GameClient::GenericComponentUpdate(icon7::ByteReader *reader)
{
	uint64_t serverId, localId;
	while (reader->get_remaining_bytes() > 9) {
		localId = serverId = 0;

		reader->op(serverId);

		auto it = mapServerEntityIdToLocalEntityId.find(serverId);
		if (it == mapServerEntityIdToLocalEntityId.end()) {
			localId = realm->NewEntity();
			mapServerEntityIdToLocalEntityId[serverId] = localId;
			mapLocalEntityIdToServerEntityId[localId] = serverId;
			RequestSpawnOf(serverId);
		} else {
			localId = it->second;
		}

		flecs::entity entity = realm->Entity(localId);
		reg::Registry::Singleton().DeserializeTemporalAllEntityComponents(
			realm, entity, *reader);
	}
}

void GameClient::PlayDeathAndDestroyEntity(
	uint64_t serverId, ComponentModelName modelName,
	ComponentLastAuthoritativeMovementState state, ComponentName name)
{
	RemoveEntity(serverId);
	PlayDeathAndDestroyEntity_virtual(modelName, state, name);
}

void GameClient::PlayAnimation(uint64_t serverId, ComponentModelName modelName,
							   ComponentLastAuthoritativeMovementState state,
							   std::string currentAnimation,
							   int64_t animationStartTick)
{
	auto it = mapServerEntityIdToLocalEntityId.find(serverId);
	if (it == mapServerEntityIdToLocalEntityId.end()) {
		LOG_INFO("TODO: request spawn of entity here");
		return;
	}
	uint64_t localId = it->second;
	realm->AddNewAuthoritativeMovementState(localId, serverId, state);
	PlayAnimation_virtual(localId, modelName, state, currentAnimation,
						  animationStartTick);
}
