#include "../../ICon7/include/icon7/Flags.hpp"
#include "../../ICon7/include/icon7/ByteReader.hpp"
#include "../../ICon7/include/icon7/Peer.hpp"
#include "../../ICon7/include/icon7/Debug.hpp"

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
	entity.add<TagPlayerEntity>();

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

		ComponentMovementState s;
		s.pos = {-10, 0, 10};
		s.vel = {0, 0, 0};
		s.onGround = false;
		entity.set<ComponentMovementState>(s);
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
		reg::Registry::Singleton().DeserializePersistentAllEntityComponents(
			this, entity, reader);
	}

	ClientRpcProxy::SpawnStaticEntities_ForPeer(this, peer);
	ClientRpcProxy::SpawnEntities_ForPeer(this, peer);

	icon7::ByteWriter writer(1500);
	ClientRpcProxy::GenericComponentUpdate_Start(this, &writer);

	ClientRpcProxy::GenericComponentUpdate_Update<
		ComponentCharacterSheet_Ranges>(this, &writer, entity);
	ClientRpcProxy::GenericComponentUpdate_Update<
		ComponentCharacterSheet_Health>(this, &writer, entity);
	ClientRpcProxy::GenericComponentUpdate_Update<
		ComponentCharacterSheet_HealthRegen>(this, &writer, entity);
	ClientRpcProxy::GenericComponentUpdate_Update<
		ComponentCharacterSheet_LevelXP>(this, &writer, entity);
	ClientRpcProxy::GenericComponentUpdate_Update<
		ComponentCharacterSheet_Strength>(this, &writer, entity);
	ClientRpcProxy::GenericComponentUpdate_Update<
		ComponentCharacterSheet_AttackCooldown>(this, &writer, entity);
	ClientRpcProxy::GenericComponentUpdate_Update<
		ComponentCharacterSheet_Protection>(this, &writer, entity);

	ClientRpcProxy::GenericComponentUpdate_Finish(this, peer, &writer);
}

void RealmServer::DisconnectPeer(icon7::Peer *peer)
{
	if (peer == nullptr) {
		LOG_ERROR("peer == nullptr");
		return;
	}
	PeerData *data = ((PeerData *)(peer->userPointer));
	if (data == nullptr) {
		LOG_ERROR("peer->userPointer is nullptr when shouldn't");
		return;
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
					entity.try_get<ComponentMovementState>();
				if (_ls) {
					auto ls = *_ls;
					ls.pos = data->nextRealmPosition;
					ls.onGround = false;

					entity.set<ComponentMovementState>(ls);
				}
			}
			if (auto cpcp = entity.try_get_mut<ComponentPlayerConnectionPeer>()) {
				cpcp->peer = nullptr;
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
		writer.op(data->nextRealm);
		reg::Registry::Singleton().SerializePersistentEntity(this, entity,
															 writer);
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
