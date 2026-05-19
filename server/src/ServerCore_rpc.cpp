#include "../../thirdparty/flecs/distr/flecs.h"

#include "../../ICon7/include/icon7/Peer.hpp"
#include "../../ICon7/include/icon7/Flags.hpp"
#include "../../ICon7/include/icon7/Debug.hpp"
#include "../../ICon7/include/icon7/Command.hpp"
#include "../../ICon7/include/icon7/CommandExecutionQueue.hpp"
#include "../../ICon7/include/icon7/CoroutineHelper.hpp"

#include "../include/ClientRpcProxy.hpp"
#include "../include/PeerStateTransitions.hpp"

#include "../include/ServerCore.hpp"

icon7::CommandExecutionQueue *ServerCore::SelectExecutionQueueByRealm(
	icon7::MessageConverter *messageConverter, icon7::PeerHandle peer,
	icon7::ByteReader &reader, icon7::Flags flags)
{
	std::shared_ptr<PeerData> &data = peerHandleToData[peer];
	if (data == nullptr) {
		LOG_ERROR("Peer's PeerData is nullptr");
		return nullptr;
	}
	std::shared_ptr<RealmServer> realm = data->realm.lock();
	if (realm) {
		return &realm->executionQueue;
	}
	return nullptr;
}

void ServerCore::Login(icon7::PeerHandle peer, const std::string &userName)
{
	std::shared_ptr<PeerData> data = peerHandleToData[peer];
	if (data) {
		peer_transitions::OnReceivedLogin(this, peer, userName);
	} else {
		LOG_ERROR("Peer received Login request but PeerData is NULL");
	}
}

void ServerCore::UpdatePlayer(icon7::CommandExecutionQueue *queue,
	icon7::PeerHandle peer, const ComponentMovementState &state)
{
	RealmServer *realm = nullptr;
	if (queue == nullptr) {
		LOG_WARN("Queue is nullptr in ServerCore::UpdatePlayer");
		std::shared_ptr<icon7::Peer> p = peer.GetSharedPeer();
		if (p == nullptr) {
			LOG_ERROR("PeerHandle points to disconnected peer.");
			return;
		}
		std::shared_ptr<PeerData> data = std::static_pointer_cast<PeerData>(p->sharedUserPointer);
		if (data == nullptr) {
			LOG_ERROR("Peer data is nullptr.");
			return;
		}
		std::shared_ptr<RealmServer> _realm = data->realm.lock();
		if (_realm == nullptr) {
			LOG_WARN("Peer is not connected to realm.");
			return;
		}
		if (!_realm->IsRunningOnCurrentThread()) {
			LOG_FATAL("Trying to update peer from other than current realm's thread.");
			LOG_WARN("TODO: implement transfering to proper realm.");
			return;
		}
		realm = _realm.get();
	} else {
		realm = std::static_pointer_cast<RealmServer>(queue->userSmartPtr).get();
		if (realm == nullptr) {
			LOG_WARN("Peer is not connected to realm.");
			return;
		}
		assert(realm->IsRunningOnCurrentThread());
	}
	assert(realm != nullptr);
	
	auto it = realm->peersData.find(peer);
	if (it == realm->peersData.end()) {
		LOG_FATAL("UpdatePlayer executed for peer on wrong realm's thread.");
		LOG_WARN("TODO: implement transfering to proper realm.");
		return;
	}
	PeerData *data = it->second.get();
	if (data == nullptr) {
		LOG_FATAL("RealmServer::peersData[peerHandle] present, but nullptr.");
		return;
	}
	
	flecs::entity entity = realm->Entity(data->entityId);
	if (entity.is_alive()) {
		// TODO: verify movement state to prevent cheating (here implement
		//       anti-cheat)

		if (auto s = entity.try_get<ComponentMovementState>()) {
			if (glm::length(s->pos - state.pos) < 5) {
				realm->currentlyUpdatingPlayerPeerEntityMovement = true;
				entity.set<ComponentMovementState>(state);
				realm->currentlyUpdatingPlayerPeerEntityMovement = false;
			} else {
				entity.set<ComponentMovementState>({*s});
				ClientRpcProxy::SpawnPlayerEntity_ForPlayer(realm, peer);
			}
		} else {
			LOG_FATAL(
				"Player entity %lu does not have ComponentMovementState",
				data->entityId);
		}
	}
}

icon7::CoroutineSchedulable ServerCore::ConnectPeerToRealm(icon7::PeerHandle peer)
{
	std::shared_ptr<icon7::Peer> _peerHolder = peer->shared_from_this();
	// TODO: replace ServerCore::ConnectPeerToRealm with something suitable to
	//       use with database
	PeerData *data = ((PeerData *)(peer->userPointer));
	if (data->userName == "") {
		LOG_INFO("Invalid usernamne");
		co_return;
	}

	std::string newRealm = data->nextRealm;
	std::string oldRealm;
	{
		std::shared_ptr<RealmServer> olr = data->realm.lock();
		if (olr.get()) {
			oldRealm = olr->realmName;
		}
	}

_LABEL_BEGINING_CONNECT_PEER_TO_REALM:

	if (oldRealm == newRealm) {
		if (data->useNextRealmPosition) {
			auto awaitable = ScheduleInRealm(data->realm);
			if (awaitable.IsValid() == false) {
				LOG_FATAL("Trying to teleport player within deleted realm.");
				co_return;
			}
			co_await awaitable;

			std::shared_ptr<RealmServer> realm = data->realm.lock();
			PeerData *data = ((PeerData *)(peer->userPointer));
			if (realm && realm == data->realm.lock()) {
				flecs::entity entity = realm->Entity(data->entityId);
				if (entity.is_alive()) {

					if (data->useNextRealmPosition) {

						data->useNextRealmPosition = false;
						auto *_ls =
							entity
								.try_get<ComponentMovementState>();
						if (_ls) {
							auto ls = *_ls;
							ls.pos = data->nextRealmPosition;
							ls.onGround = false;

							entity.set<ComponentMovementState>(ls);
						}

						ClientRpcProxy::SpawnPlayerEntity_ForPlayer(realm.get(),
																	peer);
					}
				}
			}
			co_return;
		}
		co_return;
	} else if (data->realm.lock().get()) {
		auto awaitable = ScheduleInRealm(data->realm);
		if (awaitable.IsValid() == false) {
			LOG_FATAL("Trying to teleport player within deleted realm. "
					  "Solution not implemented!!!");
			co_return;
		}
		co_await awaitable;

		std::shared_ptr<RealmServer> _oldRealm = data->realm.lock();
		if (_oldRealm) {
			_oldRealm->DisconnectPeer(peer);
			goto _LABEL_BEGINING_CONNECT_PEER_TO_REALM;
		} else {
			LOG_FATAL(
				"Current realm `%s` of player residence is not loaded (maybe "
				"got unloaded after initialising transfer of player entity to "
				"new realm). Solution not implemented!!!",
				oldRealm.c_str());
		}
		co_return;
	} else {
		std::weak_ptr<RealmServer> _w_newRealm =
			realmManager.GetRealm(newRealm);

		if (_w_newRealm.lock().get() == nullptr) {
			LOG_FATAL("New realm `%s` for player is not loaded. Solution not "
					  "implemented!!!",
					  newRealm.c_str());
			co_return;
		}

		auto awaitable = ScheduleInRealm(_w_newRealm);
		if (awaitable.IsValid() == false) {
			LOG_FATAL("Trying to teleport player within deleted realm.");
			co_return;
		}
		co_await awaitable;

		auto r = _w_newRealm.lock();
		if (r) {
			r->ConnectPeer(peer);
		} else {
			LOG_FATAL("Realm object destroy befor finished connecing player");
		}
	}
}

void ServerCore::RequestSpawnEntities(icon7::PeerHandle peer,
									  icon7::ByteReader *reader)
{
	PeerData *data = ((PeerData *)(peer->userPointer));
	std::shared_ptr<RealmServer> realm = data->realm.lock();
	if (realm && reader) {
		ClientRpcProxy::SpawnEntities_ForPeerByIds(realm.get(), peer, *reader);
	}
}

void ServerCore::InteractInLineOfSight(
	icon7::PeerHandle peer, ComponentMovementState state,
	uint64_t targetId, glm::vec3 dstPos, glm::vec3 normal)
{
	// TODO: add server side verification of raycast
	PeerData *data = ((PeerData *)(peer->userPointer));
	std::shared_ptr<RealmServer> realm = data->realm.lock();
	if (realm) {
		UpdatePlayer(peer, state);
		flecs::entity entity = realm->Entity(data->entityId);
		if (entity.is_alive()) {
			auto s = entity.try_get<ComponentMovementState>();
			if (s) {
				state = *s;
			}
		}
		realm->InteractInLineOfSight(peer, state, targetId, dstPos, normal);
	}
}

void ServerCore::Attack(icon7::PeerHandle peer,
						ComponentMovementState state,
						uint64_t targetId, glm::vec3 targetPos,
						int64_t attackType, int64_t attackId, int64_t argInt)
{
	PeerData *data = ((PeerData *)(peer->userPointer));
	std::shared_ptr<RealmServer> realm = data->realm.lock();
	if (realm) {
		UpdatePlayer(peer, state);
		flecs::entity entity = realm->Entity(data->entityId);
		if (entity.is_alive()) {
			auto s = entity.try_get<ComponentMovementState>();
			if (s) {
				state = *s;
			}
		}
		realm->Attack(peer, state, targetId, targetPos, attackType, attackId,
					  argInt);
	}
}
