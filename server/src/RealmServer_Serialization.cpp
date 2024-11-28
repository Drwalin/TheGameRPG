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

void RealmServer::SaveAllEntitiesToFiles()
{
	SaveNonPlayerEntitiesToFile();
	SavePlayerEntitiesToFiles();
}

void RealmServer::SaveNonPlayerEntitiesToFile()
{
	icon7::ByteWriter writer(1024 * 1024);
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
		entity.get_mut<ComponentPlayerConnectionPeer>();
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
	// TODO: Implement database write waiting
	LOG_TRACE("Waiting for flushed database write is not implemented.");
}
