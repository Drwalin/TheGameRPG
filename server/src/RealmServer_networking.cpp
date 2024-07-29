#include <icon7/Flags.hpp>
#include <icon7/ByteReader.hpp>
#include <icon7/Peer.hpp>
#include <icon7/Debug.hpp>

#include "../../common/include/EntitySystems.hpp"
#include "../../common/include/ComponentCharacterSheet.hpp"
#include "../../common/include/RegistryComponent.hpp"

#include "../include/ServerCore.hpp"
#include "../include/ClientRpcProxy.hpp"
#include "../include/EntityNetworkingSystems.hpp"
#include "../include/FileOperations.hpp"

#include "../include/RealmServer.hpp"

void RealmServer::ConnectPeer(icon7::Peer *peer)
{
	PeerData *data = ((PeerData *)(peer->userPointer));
	data->realm = weak_from_this();

	uint64_t entityId = NewEntity();

	flecs::entity entity = Entity(entityId);

	auto pw = peer->shared_from_this();
	peers[pw] = entityId;
	SetComponent<ComponentPlayerConnectionPeer>(
		entityId, ComponentPlayerConnectionPeer{peer->shared_from_this()});
	data->entityId = entityId;

	icon7::ByteReader reader(data->storedEntityData, 0);
	if (!(data->storedEntityData.valid() &&
		  data->storedEntityData.size() > 3) &&
		(reader.has_any_more() == false || reader.is_valid() == false)) {
		entity.add<ComponentShape>();
		entity.add<ComponentMovementParameters>();
		ComponentModelName modelName;
		modelName.modelName =
			"characters/low_poly_medieval_people/city_dwellers_1_model.tscn";
		entity.set<ComponentModelName>(modelName);
		entity.add<ComponentEventsQueue>();

		ComponentLastAuthoritativeMovementState s;
		s.oldState.timestamp = timer.currentTick;
		s.oldState.pos = {-87, 0, 54};
		s.oldState.vel = {0, 0, 0};
		s.oldState.onGround = false;
		entity.set<ComponentLastAuthoritativeMovementState>(s);
		entity.set<ComponentMovementState>(s.oldState);
		entity.add<ComponentCharacterSheet_Ranges>();
		entity.add<ComponentCharacterSheet_Health>();
		entity.add<ComponentCharacterSheet_HealthRegen>();
		entity.add<ComponentCharacterSheet_LevelXP>();
		entity.add<ComponentCharacterSheet_Strength>();
		entity.add<ComponentCharacterSheet_AttackCooldown>();
		entity.add<ComponentCharacterSheet_Protection>();

		ComponentName name;
		name.name = data->userName;
		SetComponent<ComponentName>(entityId, name);
	} else {
		std::string_view sv;
		reader.op(sv);
		reg::Registry::Singleton().DeserializeAllEntityComponents(entity,
																  reader);
	}
	

	ClientRpcProxy::SpawnStaticEntities_ForPeer(this, peer);
	ClientRpcProxy::SpawnEntities_ForPeer(this, peer);

	icon7::ByteWriter writer(1500);
	ClientRpcProxy::GenericComponentUpdate_Start(this, &writer);
	
	ClientRpcProxy::GenericComponentUpdate_Update<ComponentCharacterSheet_Ranges>(this, &writer, entity);
	ClientRpcProxy::GenericComponentUpdate_Update<ComponentCharacterSheet_Health>(this, &writer, entity);
	ClientRpcProxy::GenericComponentUpdate_Update<ComponentCharacterSheet_HealthRegen>(this, &writer, entity);
	ClientRpcProxy::GenericComponentUpdate_Update<ComponentCharacterSheet_LevelXP>(this, &writer, entity);
	ClientRpcProxy::GenericComponentUpdate_Update<ComponentCharacterSheet_Strength>(this, &writer, entity);
	ClientRpcProxy::GenericComponentUpdate_Update<ComponentCharacterSheet_AttackCooldown>(this, &writer, entity);
	ClientRpcProxy::GenericComponentUpdate_Update<ComponentCharacterSheet_Protection>(this, &writer, entity);
	
	ClientRpcProxy::GenericComponentUpdate_Finish(this, peer, &writer);
}

void RealmServer::DisconnectPeer(icon7::Peer *peer)
{
	PeerData *data = ((PeerData *)(peer->userPointer));
	if (data == nullptr) {
		LOG_ERROR("peer->userPointer is nullptr when shouldn't");
	}
	auto pw = peer->shared_from_this();
	auto it = peers.find(pw);
	if (it != peers.end()) {
		uint64_t entityId = it->second;

		flecs::entity entity = Entity(data->entityId);
		if (entity.is_alive()) {
			if (data->useNextRealmPosition) {
				data->useNextRealmPosition = false;
				auto *_ls =
					entity.get<ComponentLastAuthoritativeMovementState>();
				if (_ls) {
					auto ls = *_ls;
					ls.oldState.pos = data->nextRealmPosition;
					ls.oldState.onGround = false;

					entity.set(ls);

					if (entity.has<ComponentMovementState>()) {
						entity.set(ls.oldState);
					}
				}
			}
		}

		peers.erase(it);

		StorePlayerDataInPeerAndFile(peer);
		RemoveEntity(entityId);
	}
	data->realm.reset();
}

void RealmServer::StorePlayerDataInPeerAndFile(icon7::Peer *peer)
{
	PeerData *data = ((PeerData *)(peer->userPointer));
	if (data == nullptr) {
		LOG_ERROR("peer->userPointer is nullptr when shouldn't");
		return;
	}
	flecs::entity entity = Entity(data->entityId);
	if (entity.is_alive()) {
		if (data->nextRealm == "") {
			auto realm = data->realm.lock();
			data->nextRealm = realm->realmName;
		}

		icon7::ByteWriter writer(std::move(data->storedEntityData));
		if (writer.Buffer().valid() == false) {
			writer.Buffer().Init(4096);
		}
		writer.Buffer().resize(0);
		// TODO: write new realm for player
		writer.op(data->nextRealm);
		reg::Registry::Singleton().SerializeEntity(entity, writer);
		data->storedEntityData = std::move(writer.Buffer());

		std::string filePrefix;
		if (serverCore->configStorage.GetString(
				"config.users_directory_storage.prefix", &filePrefix)) {
		}
		if (FileOperations::WriteFile(filePrefix + data->userName,
									  data->storedEntityData)) {
		} else {
			data->storedEntityData.clear();
		}
	}
}

void RealmServer::Broadcast(icon7::ByteBuffer &buffer, uint64_t exceptEntityId)
{
	for (auto it : peers) {
		if (it.second != exceptEntityId) {
			if (((PeerData *)(it.first->userPointer))->entityId !=
				exceptEntityId) {
				it.first->Send(buffer);
			}
		}
	}
}
