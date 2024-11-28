#include <filesystem>

#include <icon7/Flags.hpp>
#include <icon7/ByteReader.hpp>

#include "../../common/include/EntitySystems.hpp"
#include "../../common/include/RegistryComponent.hpp"
#include "../../common/include/CollisionLoader.hpp"

#define ENABLE_REALM_SERVER_IMPLEMENTATION_TEMPLATE

#include "../include/ServerCore.hpp"
#include "../include/ClientRpcProxy.hpp"
#include "../include/EntityNetworkingSystems.hpp"
#include "../include/FileOperations.hpp"
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
	// TODO: safe all entities and states
	std::unordered_map<std::shared_ptr<icon7::Peer>, uint64_t> p = peers;
	for (auto it : p) {
		DisconnectPeer(it.first.get());
	}
}

uint64_t RealmServer::NewEntity()
{
	uint64_t id = Realm::NewEntity();
	Entity(id).add<TagNonPlayerEntity>();
	return id;
}

void RealmServer::Init(const std::string &realmName)
{
	// TODO: load static realm data from database/disk
	Realm::Init(realmName);

	LoadFromFile();

	RegisterGameLogic();

	sendEntitiesToClientsTimer = 0;
}

std::string RealmServer::GetReadFileName()
{
	const std::string fileNameSecond = GetWriteFileName();

	if (std::filesystem::exists(fileNameSecond)) {
		return fileNameSecond;
	}

	std::string filePrefix, fileSuffix;
	if (serverCore->configStorage.GetString(
			"config.realm_map_file.file_path.prefix", &filePrefix)) {
	}
	if (serverCore->configStorage.GetString(
			"config.realm_map_file.file_path.suffix", &fileSuffix)) {
	}
	const std::string fileName = filePrefix + realmName + fileSuffix;
	return fileName;
}

std::string RealmServer::GetWriteFileName()
{
	std::string filePrefix, fileSuffixSecond;
	if (serverCore->configStorage.GetString(
			"config.realm_map_file.file_path.prefix", &filePrefix)) {
	}
	if (serverCore->configStorage.GetString(
			"config.realm_map_file.file_path.suffix_saved_file",
			&fileSuffixSecond)) {
	}
	return filePrefix + realmName + fileSuffixSecond;
}

bool RealmServer::LoadFromFile()
{
	std::string fileName = GetReadFileName();
	icon7::ByteBuffer buffer;
	if (FileOperations::ReadFile(fileName, &buffer)) {
		LOG_INFO("Start loading realm: '%s'  (%s)", realmName.c_str(),
				 fileName.c_str());
		icon7::ByteReader reader(buffer, 0);
		while (reader.get_remaining_bytes() > 10) {
			uint64_t entityId = NewEntity();
			flecs::entity entity = Entity(entityId);
			reg::Registry::Singleton().DeserializePersistentAllEntityComponents(
				this, entity, reader);
			if (reader.is_valid() == false) {
				RemoveEntity(entity);
				break;
			}
		}
		System([this](flecs::entity entity, ComponentTrigger &trigger) {
			trigger.Tick(entity.id(), this);
		});
		LOG_INFO("Finished loading realm: '%s'", realmName.c_str());
		return true;
	} else {
		LOG_ERROR("Failed to open map file: '%s'", fileName.c_str());
		return false;
	}
}

bool RealmServer::SaveEntitesToFiles() {}

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
	// TODO: here do other server updates, AI, other mechanics and logic,
	// defer some work to other worker threads (ai, db)  maybe?

	if (sendEntitiesToClientsTimer + sendUpdateDeltaTicks <=
		timer.currentTick) {
		sendEntitiesToClientsTimer = timer.currentTick;
		ClientRpcProxy::Broadcast_UpdateEntities(this);
		return true;
	} else {
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
