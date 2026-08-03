#include "../../ICon7/include/icon7/ByteReader.hpp" // IWYU pragma: export
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

void RealmServer::ConnectPeer(icon7::PeerHandle peer)
{
	assert(peersData_.Get(peer) == nullptr);
	std::shared_ptr<icon7::Peer> p = peer.GetSharedPeer();
	PeerEntryData &data= peersData_.Access(peer);
	assert(data.data == nullptr);
	data.data = std::static_pointer_cast<PeerData>(p->sharedUserPointer);
	
	data.data->realm = weak_from_this();

	uint64_t entityId = NewEntity();
	flecs::entity entity = Entity(entityId);
	entity.add<TagPlayerEntity>();

	SetComponent<ComponentPlayerConnectionPeer>(
entityId, ComponentPlayerConnectionPeer{p});
	data.data->entityId = entityId;
	data.entity = entityId;

	icon7::ByteReaderBase reader(data.data->storedEntityData.data(), 0,
			data.data->storedEntityData.size());
// 	icon7::ByteReader reader(data.data->storedEntityData, 0);
	if (!(data.data->storedEntityData.valid() &&
		  data.data->storedEntityData.size() > 3) &&
		(reader.has_any_more() == false || reader.is_valid() == false)) {
		entity.add<ComponentShape>();
		entity.add<ComponentMovementParameters>();
		ComponentModelName modelName;
		modelName.modelName =
			"characters/low_poly_medieval_people/city_dwellers_1_model.tscn";
		entity.set<ComponentModelName>(modelName);
		entity.add<ComponentEventsQueue>();

		ComponentMovementState s;
		s.pos = {-7.8, 0, 7.8};
		s.vel = {0, 0, 0};
		s.rot = {0, 3.141592f, 0};
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
		name.name = data.data->userName;
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

void RealmServer::DisconnectPeer(icon7::PeerHandle peer)
{
	if (!peer) {
		LOG_ERROR("trying to disconnect invalid peer");
		return;
	}

	PeerEntryData *data = peersData_.Get(peer);
	if (data == nullptr) {
		LOG_FATAL("Cannot operate on peer with no stored PeerEntryData");
		return;
	}

	if (data->data == nullptr) {
		LOG_ERROR("PeerEntryData->data is nullptr when shouldn't");
		return;
	}
	if (data != nullptr) {
		uint64_t entityId = data->entity;

		flecs::entity entity = Entity(entityId);
		if (entity.is_alive()) {
			if (data->data->useNextRealmPosition) {
				data->data->useNextRealmPosition = false;
				auto *_ls =
					entity.try_get<ComponentMovementState>();
				if (_ls) {
					auto ls = *_ls;
					ls.pos = data->data->nextRealmPosition;
					ls.onGround = false;

					entity.set<ComponentMovementState>(ls);
				}
			}
			if (auto cpcp = entity.try_get_mut<ComponentPlayerConnectionPeer>()) {
				cpcp->peer = nullptr;
			}
		}

		StorePlayerDataInPeerAndFile(peer);
		RemoveEntity(entityId);
	}
	data->data->realm.reset();
	peersData_.Remove(peer);
}

void RealmServer::StorePlayerDataInPeerAndFile(icon7::PeerHandle peer)
{
	PeerEntryData *d = peersData_.Get(peer);
	if (d == nullptr) {
		LOG_FATAL("Cannot operate on peer with no stored PeerEntryData");
		return;
	}
	PeerData *data = d->data.get();

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
		writer.Reinit(4064);
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

void RealmServer::Broadcast(icon7::ByteBufferReadable &buffer, uint64_t exceptEntityId)
{
	for (auto &ita : peersData_.maps.data) {
		for (auto &it : ita.denseData) {
			if (it.entity != exceptEntityId) {
				it.data->peerHandle.Send(buffer);
			}
		}
	}
}
