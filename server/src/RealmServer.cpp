#include "../../common/include/EntitySystems.hpp"

#define ENABLE_REALM_SERVER_IMPLEMENTATION_TEMPLATE

#include "../include/ServerCore.hpp"
#include "../include/ClientRpcProxy.hpp"
#include "../include/EntityNetworkingSystems.hpp"
#include "../include/EntityGameComponents.hpp"

#include "../include/RealmServer.hpp"

int RegisterEntityGameComponents(flecs::world &ecs);

RealmServer::RealmServer()
{
	RealmServer::RegisterObservers();
	RegisterEntityGameComponents(ecs);
}

RealmServer::~RealmServer() { DisconnectAllAndDestroy(); }

void RealmServer::QueueDestroy() { queueDestroy = true; }

bool RealmServer::IsQueuedToDestroy() { return queueDestroy; }

void RealmServer::DisconnectAllAndDestroy()
{
	std::unordered_map<std::shared_ptr<icon7::Peer>, uint64_t> p = peers;
	for (auto it : p) {
		DisconnectPeer(it.first.get());
	}

	SaveNonPlayerEntitiesToFile();

	FlushSavingData();
	WaitForFlushedData();
}

uint64_t RealmServer::NewEntity()
{
	uint64_t id = Realm::NewEntity();
	Entity(id).add<TagNonPlayerEntity>();
	return id;
}

void RealmServer::Init(const std::string &realmName)
{
	millisecondsBetweenStatsReport =
		icon7::time::milliseconds(serverCore->configStorage.GetOrSetInteger(
			"metrics.realm.milliseconds_between_stats_report", 60000));

	executionQueue.userSmartPtr = this->shared_from_this();

	// TODO: load static realm data from database/disk
	Realm::Init(realmName);

	LoadFromFile();

	sendEntitiesToClientsTimer = {0};
}

void RealmServer::OneEpoch()
{
	executionQueue.Execute(16384);
	Realm::OneEpoch();

	// TODO: replace with observer inside collision world on collision
	ecs.each([this](flecs::entity entity, ComponentTrigger &trigger) {
		trigger.Tick(entity.id(), this);
	});

	// TODO: here do other server updates, AI, other mechanics and logic, defer
	//       some work to other worker threads (ai, db)  maybe?

	if (nextTickToSaveAllDataToFiles <= timer.currentTick) {
		SaveAllEntitiesToFiles();
	}

	if (sendEntitiesToClientsTimer + sendUpdateDeltaTicks <=
		timer.currentTick) {
		sendEntitiesToClientsTimer = timer.currentTick;
		ClientRpcProxy::Broadcast_UpdateEntities(this);
	}
	FlushSavingData();
}

void RealmServer::ExecuteOnRealmThread(
	icon7::CommandHandle<icon7::Command> &&command)
{
	executionQueue.EnqueueCommand(std::move(command));
}

void RealmServer::ExecuteMovementUpdate(uint64_t entityId,
										ComponentMovementState *state)
{
	auto entity = Entity(entityId);

	const ComponentShape *shape = entity.try_get<ComponentShape>();
	if (shape == nullptr) {
		*state = {};
		return;
	}
	ComponentMovementState *currentState =
		(ComponentMovementState *)entity.try_get<ComponentMovementState>();
	if (currentState == nullptr) {
		*state = {};
		return;
	}
	const ComponentLastAuthoritativeMovementState *lastAuthoritativeState =
		entity.try_get<ComponentLastAuthoritativeMovementState>();
	if (lastAuthoritativeState == nullptr) {
		*state = {};
		return;
	}
	const ComponentMovementParameters *movementParams =
		entity.try_get<ComponentMovementParameters>();
	if (movementParams == nullptr) {
		*state = {};
		return;
	}

	EntitySystems::UpdateMovement(this, entity, *shape, *currentState,
								  *lastAuthoritativeState, *movementParams);

	*state = *currentState;
}
