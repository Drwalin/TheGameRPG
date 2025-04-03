#include <icon7/Flags.hpp>
#include <icon7/ByteReader.hpp>

#include "../../common/include/EntitySystems.hpp"
#include "../../common/include/CollisionLoader.hpp"

#define ENABLE_REALM_SERVER_IMPLEMENTATION_TEMPLATE

#include "../include/ServerCore.hpp"
#include "../include/ClientRpcProxy.hpp"
#include "../include/EntityNetworkingSystems.hpp"

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
	millisecondsBetweenStatsReport = serverCore->configStorage.GetOrSetInteger(
		"metrics.realm.milliseconds_between_stats_report", 60000);

	executionQueue.userSmartPtr = this->shared_from_this();

	// TODO: load static realm data from database/disk
	Realm::Init(realmName);

	LoadFromFile();

	RegisterGameLogic();

	sendEntitiesToClientsTimer = 0;
}

bool RealmServer::GetCollisionShape(std::string collisionShapeName,
									TerrainCollisionData *data)
{
	std::string filePrefix;
	if (serverCore->configStorage.GetString(
			"config.collision_shape.file_path.prefix", &filePrefix)) {
		collisionShapeName = filePrefix + collisionShapeName;
	}
	CollisionLoader loader;
	bool res = loader.LoadOBJ((const std::string &)collisionShapeName);
	*data = std::move(loader.collisionData);
	return res && data->vertices.size() >= 3 && data->indices.size() >= 3;
}

bool RealmServer::OneEpoch()
{
	bool busy = executionQueue.Execute(128) != 0;
	busy |= Realm::OneEpoch();
	// TODO: here do other server updates, AI, other mechanics and logic, defer
	//       some work to other worker threads (ai, db)  maybe?

	if (nextTickToSaveAllDataToFiles <= timer.currentTick) {
		SaveAllEntitiesToFiles();
	}

	if (sendEntitiesToClientsTimer + sendUpdateDeltaTicks <=
		timer.currentTick) {
		sendEntitiesToClientsTimer = timer.currentTick;
		ClientRpcProxy::Broadcast_UpdateEntities(this);
		FlushSavingData();
		return true;
	} else {
		FlushSavingData();
		return busy;
	}
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

	const ComponentShape *shape = entity.get<ComponentShape>();
	if (shape == nullptr) {
		*state = {};
		return;
	}
	ComponentMovementState *currentState =
		(ComponentMovementState *)entity.get<ComponentMovementState>();
	if (currentState == nullptr) {
		*state = {};
		return;
	}
	const ComponentLastAuthoritativeMovementState *lastAuthoritativeState =
		entity.get<ComponentLastAuthoritativeMovementState>();
	if (lastAuthoritativeState == nullptr) {
		*state = {};
		return;
	}
	const ComponentMovementParameters *movementParams =
		entity.get<ComponentMovementParameters>();
	if (movementParams == nullptr) {
		*state = {};
		return;
	}

	EntitySystems::UpdateMovement(this, entity, *shape, *currentState,
								  *lastAuthoritativeState, *movementParams);

	*state = *currentState;
}
