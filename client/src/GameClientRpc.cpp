#include "../../ICon7/include/icon7/ByteReader.hpp"
#include "../../ICon7/include/icon7/Debug.hpp"

#include "../include/ServerRpcProxy.hpp"
#include "../include/RegistryComponent.hpp"
#include "../include/EntityComponentsClient.hpp"

#include "../include/GameClient.hpp"

void GameClient::JoinRealm(const std::string &realmName, Tick currentTick,
						   uint64_t playerEntityId)
{
	auto a = TickTimer::GetCurrentTimepoint();
	if (serverPlayerEntityId || localPlayerEntityId) {
		OnPlayerIdUnset();
		serverPlayerEntityId = 0;
		localPlayerEntityId = 0;
	}
	realm->timer.Start(currentTick, realm->tickDuration);
	realm->Reinit(realmName);
	SetPlayerEntityId(playerEntityId);
	auto b = TickTimer::GetCurrentTimepoint();
	realm->timer.Start(currentTick + ((b-a).ns / realm->tickDuration.ns), realm->tickDuration);
}

void GameClient::SpawnEntities(icon7::ByteReader *reader)
{
	uint64_t serverId;
	ComponentMovementState state;
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
	ComponentCollisionShape shape;
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

void GameClient::UpdateEntities(Tick tick, icon7::ByteReader *reader)
{
	tick += RealmClient::TICKS_UPDATE_DELAY;
	uint64_t serverId;
	ComponentMovementState state;
	while (reader->get_remaining_bytes() > 8) {
		reader->op(serverId);
		reader->op(state);
		
		if (reader->is_valid()) {
			UpdateEntity(serverId, state, tick);
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

void GameClient::Pong(Tick clientLastSentTick,
					  int64_t clientLastSentTickTimeNs,
					  Tick serverLastProcessedTick,
					  int64_t serverTickStartTimeOffsetNs,
					  icon7::time::Point clientPingSentTime)
{
	const Tick clientCurrentTick = realm->timer.currentTick;
	const icon7::time::Point clientCurrentTime =
		TickTimer::GetCurrentTimepoint();
	const icon7::time::Diff rttDuration =
		(clientCurrentTime - clientPingSentTime);
	const icon7::time::Diff oneWayLatencyDuration = rttDuration / 2;
	ping = (ping * 15 + rttDuration) / 16;
	if (serverLastProcessedTick != 0) {
		// var: describe time difference between server tick start and an
		//      instant that server responded to client
		const icon7::time::Diff serverTickStartTimeOffsetDuration(
			serverTickStartTimeOffsetNs);

		// var: time point (in this computer local time) when approximetly
		//      started tick on server before it's response
		const icon7::time::Point serverTickStartTimeClientLocal =
			clientCurrentTime -// oneWayLatencyDuration -
			serverTickStartTimeOffsetDuration;

		// var: amount of ticks between the tick on server and current next
		//      predicted local tick
		const Tick serverTicksElapsedSinceServer =
			Tick{(clientCurrentTime - serverTickStartTimeClientLocal).ns /
				realm->tickDuration.ns +
			1};// - RealmClient::TICKS_UPDATE_DELAY};

		// var: amount of ticks between the tick on server and current next
		//      predicted local tick with included delay (client simulation is
		//      delayed by RealmClient::TICKS_UPDATE_DELAY)
		const Tick ticksToNextDelayedFromServer =
			serverTicksElapsedSinceServer;// - RealmClient::TICKS_UPDATE_DELAY;

		// var: amount of ticks between the tick on server and current next
		//      local tick with included delay (client simulation is delayed by
		//      RealmClient::TICKS_UPDATE_DELAY)
		const Tick targetServerTickAdvance = clientCurrentTick -
												serverLastProcessedTick +
												ticksToNextDelayedFromServer;

		// var: time point (in local time) at which next delayed tick should be
		const icon7::time::Point targetNextTickTimeClientLocal =
			serverTickStartTimeClientLocal +
			realm->tickDuration * targetServerTickAdvance.v;

		// var: correctional delta of local timer delay
		const icon7::time::Diff timeCorrectionDelta =
			targetNextTickTimeClientLocal - realm->timer.nextTick;
		const icon7::time::Diff timeCorrectionDeltaClamped = icon7::time::clamp(
			timeCorrectionDelta, -RealmClient::BASE_MAX_CORRECTION_OF_NEXT_TICK,
			RealmClient::BASE_MAX_CORRECTION_OF_NEXT_TICK);

		const icon7::time::Diff timeCorrectionDeltaAbs =
			abs(timeCorrectionDelta);

		// Correcting local timer
		realm->timer.nextTick -= timeCorrectionDeltaClamped;

		// Requesting next ping time correction when client is unsyncronised
		if (timeCorrectionDeltaAbs > realm->tickDuration) {
			ServerRpcProxy::Ping(this, false);
		}
	}
}

void GameClient::SpawnEntity(
	uint64_t serverId, const ComponentMovementState &state,
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
								   ComponentCollisionShape shape)
{
	uint64_t localId = realm->CreateStaticEntity(transform, model, shape);
	mapServerEntityIdToLocalEntityId[serverId] = localId;
	mapLocalEntityIdToServerEntityId[localId] = serverId;
}

void GameClient::UpdateEntity(
	uint64_t serverId, const ComponentMovementState state, Tick tick)
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
						 icon7::time::GetTemporaryTimestamp(),
						 realm->timer.currentTick, tick});
		realm->AddNewAuthoritativeMovementState(localId, serverId, state, tick);
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
	ComponentMovementState state, ComponentName name)
{
	RemoveEntity(serverId);
	PlayDeathAndDestroyEntity_virtual(modelName, state, name);
}

void GameClient::PlayAnimation(uint64_t serverId, ComponentModelName modelName,
							   ComponentMovementState state,
							   std::string currentAnimation,
							   Tick animationStartTick)
{
	animationStartTick += RealmClient::TICKS_UPDATE_DELAY;
	auto it = mapServerEntityIdToLocalEntityId.find(serverId);
	if (it == mapServerEntityIdToLocalEntityId.end()) {
		LOG_INFO("TODO: request spawn of entity here");
		return;
	}
	uint64_t localId = it->second;
	realm->AddNewAuthoritativeMovementState(localId, serverId, state, animationStartTick);
	PlayAnimation_virtual(localId, modelName, state, currentAnimation,
						  animationStartTick);
}
