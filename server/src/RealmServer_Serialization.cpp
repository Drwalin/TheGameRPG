#include <filesystem>

#include "../../ICon7/include/icon7/Flags.hpp"
#include "../../ICon7/include/icon7/ByteReader.hpp"

#include "../../common/include/RegistryComponent.hpp"

#define ENABLE_REALM_SERVER_IMPLEMENTATION_TEMPLATE

#include "../include/ServerCore.hpp"
#include "../include/ClientRpcProxy.hpp"
#include "../include/EntityNetworkingSystems.hpp"
#include "../include/FileOperations.hpp"

#include "../include/RealmServer.hpp"

std::string RealmServer::GetReadFileName(bool *isFromSavedState)
{
	const std::string fileNameSecond = GetWriteFileName();

	if (std::filesystem::exists(fileNameSecond)) {
		*isFromSavedState = true;
		return fileNameSecond;
	}
	*isFromSavedState = false;

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
	int64_t startingTimerTick = 0;
	bool isFromSavedState = false;
	std::string fileName = GetReadFileName(&isFromSavedState);
	icon7::ByteBuffer buffer;
	if (FileOperations::ReadFile(fileName, &buffer)) {
		LOG_INFO("Start loading realm: '%s'  (%s)", realmName.c_str(),
				 fileName.c_str());
		icon7::ByteReader reader(buffer, 0);
		if (isFromSavedState) {
			reader.op(startingTimerTick);
		}
		timer.Start(startingTimerTick);
		while (reader.get_remaining_bytes() > 9) {
			uint64_t entityId = NewEntity();
			flecs::entity entity = Entity(entityId);
			reg::Registry::Singleton().DeserializePersistentAllEntityComponents(
				this, entity, reader);
			if (reader.is_valid() == false) {
				RemoveEntity(entity);
				break;
			}
		}
		timer.Start(startingTimerTick);
		LOG_INFO("Finished loading realm: '%s'", realmName.c_str());
		return true;
	} else {
		LOG_ERROR("Failed to open map file: '%s'", fileName.c_str());
		return false;
	}
	SetNextTickFotSavingData();
}

void RealmServer::SaveAllEntitiesToFiles()
{
	SaveNonPlayerEntitiesToFile();
	SavePlayerEntitiesToFiles();
	SetNextTickFotSavingData();
}

void RealmServer::SetNextTickFotSavingData()
{
	nextTickToSaveAllDataToFiles =
		timer.currentTick +
		serverCore->configStorage.GetOrSetInteger(
			"config.milliseconds_between_saving_all_data_to_database",
			1000 * 60 * 5);
}

void RealmServer::SaveNonPlayerEntitiesToFile()
{
	icon7::ByteWriter writer(1024 * 1024);
	writer.op(timer.currentTick);
	ecs.each([&](flecs::entity entity, TagNonPlayerEntity) {
		reg::Registry::Singleton().SerializePersistentEntity(this, entity,
															 writer);
	});
	std::string fileName = GetWriteFileName();
	FileOperations::WriteFile(fileName, writer.Buffer());
}

void RealmServer::SavePlayerEntitiesToFiles()
{
	ecs.each([this](flecs::entity entity, TagPlayerEntity) {
		SavePlayerEntityToFile(entity);
	});
}

void RealmServer::SavePlayerEntityToFile(flecs::entity entity)
{
	ComponentPlayerConnectionPeer *peer =
		entity.try_get_mut<ComponentPlayerConnectionPeer>();
	if (peer) {
		if (peer->peer.get() == nullptr) {
			LOG_FATAL(
				"ComponentPlayerConnectionPeer of entity %lu has nullptr value",
				entity);
		} else {
			StorePlayerDataInPeerAndFile(peer->peer.get());
		}
	} else {
		LOG_FATAL(
			"Player entity %lu does not have ComponentPlayerConnectionPeer",
			entity);
	}
}

void RealmServer::FlushSavingData()
{
	// TODO: implement database write flush
}

void RealmServer::WaitForFlushedData()
{
	// TODO: Implement database write sync/wait
	LOG_TRACE("Waiting for flushed database write is not implemented.");
}
