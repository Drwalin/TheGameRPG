#include <flecs.h>

#include <icon7/Peer.hpp>
#include <icon7/Flags.hpp>
#include <icon7/Debug.hpp>
#include <icon7/Command.hpp>

#include "../include/ClientRpcProxy.hpp"
#include "../include/PeerStateTransitions.hpp"

#include "../include/ServerCore.hpp"

icon7::CommandExecutionQueue *ServerCore::SelectExecutionQueueByRealm(
	icon7::MessageConverter *messageConverter, icon7::Peer *peer,
	icon7::ByteReader &reader, icon7::Flags flags)
{
	PeerData *data = ((PeerData *)(peer->userPointer));
	std::shared_ptr<RealmServer> realm = data->realm.lock();
	if (realm) {
		return &realm->executionQueue;
	}
	return nullptr;
}

void ServerCore::Login(icon7::Peer *peer, const std::string &userName)
{
	PeerData *data = ((PeerData *)(peer->userPointer));
	if (data) {
		peer_transitions::OnReceivedLogin(this, peer, userName);
	} else {
		LOG_ERROR("Peer received Login request but PeerData is NULL");
	}
}

void ServerCore::UpdatePlayer(
	icon7::Peer *peer, const ComponentLastAuthoritativeMovementState &state)
{
	PeerData *data = ((PeerData *)(peer->userPointer));
	std::shared_ptr<RealmServer> realm = data->realm.lock();
	if (realm) {
		flecs::entity entity = realm->Entity(data->entityId);
		if (entity.is_alive()) {
			// TODO: verify movement state to prevent cheating (here implement
			//       anti-cheat)

			if (auto s = entity.get<ComponentMovementState>()) {
				if (glm::length(s->pos - state.oldState.pos) < 5) {
					realm->currentlyUpdatingPlayerPeerEntityMovement = true;
					entity.set<ComponentLastAuthoritativeMovementState>(state);
					entity.set<ComponentMovementState>(state.oldState);
					realm->currentlyUpdatingPlayerPeerEntityMovement = false;
				} else {
					entity.set<ComponentLastAuthoritativeMovementState>({*s});
					ClientRpcProxy::SpawnPlayerEntity_ForPlayer(realm.get(),
																peer);
				}
			} else {
				LOG_FATAL(
					"Player entity %lu does not have ComponentMovementState",
					data->entityId);
			}
		}
	}
}

void ServerCore::ConnectPeerToRealm(icon7::Peer *peer)
{
	// TODO: replace ServerCore::ConnectPeerToRealm with something suitable to
	//       use with database
	PeerData *data = ((PeerData *)(peer->userPointer));
	if (data->userName == "") {
		LOG_INFO("Invalid usernamne");
		return;
	}
	std::shared_ptr<RealmServer> newRealm =
		realmManager.GetRealm(data->nextRealm);
	if (newRealm == nullptr) {
		LOG_INFO("Invalid realm");
		return;
	}

	std::shared_ptr<RealmServer> oldRealm = data->realm.lock();
	if (oldRealm == newRealm) {
		if (data->useNextRealmPosition) {
			class CommandTeleport : public icon7::commands::ExecuteOnPeer
			{
			public:
				CommandTeleport() = default;
				~CommandTeleport() = default;

				std::weak_ptr<RealmServer> realm;
				virtual void Execute() override
				{
					std::shared_ptr<RealmServer> realm = this->realm.lock();
					PeerData *data = ((PeerData *)(peer->userPointer));
					if (realm && realm == data->realm.lock()) {
						flecs::entity entity = realm->Entity(data->entityId);
						if (entity.is_alive()) {

							if (data->useNextRealmPosition) {

								data->useNextRealmPosition = false;
								auto *_ls = entity.get<
									ComponentLastAuthoritativeMovementState>();
								if (_ls) {
									auto ls = *_ls;
									ls.oldState.pos = data->nextRealmPosition;
									ls.oldState.onGround = false;

									entity.set<
										ComponentLastAuthoritativeMovementState>(
										ls);

									if (entity.has<ComponentMovementState>()) {
										entity.set<ComponentMovementState>(
											ls.oldState);
									}
								}

								ClientRpcProxy::SpawnPlayerEntity_ForPlayer(
									realm.get(), peer.get());
							}
						}
					}
				}
			};
			auto com = icon7::CommandHandle<CommandTeleport>::Create();
			com->peer = peer->shared_from_this();
			com->realm = newRealm;
			newRealm->executionQueue.EnqueueCommand(std::move(com));
		}
		return;
	} else if (oldRealm) {
		class CommandDisconnectPeer : public icon7::commands::ExecuteOnPeer
		{
		public:
			CommandDisconnectPeer() = default;
			~CommandDisconnectPeer() = default;

			std::weak_ptr<RealmServer> newRealm;
			std::weak_ptr<RealmServer> oldRealm;
			virtual void Execute() override
			{
				std::shared_ptr<RealmServer> oldRealm = this->oldRealm.lock();
				std::shared_ptr<RealmServer> newRealm = this->newRealm.lock();
				if (oldRealm) {
					oldRealm->DisconnectPeer(peer.get());

					class CommandConnectPeerToRealm
						: public icon7::commands::ExecuteOnPeer
					{
					public:
						CommandConnectPeerToRealm() = default;
						~CommandConnectPeerToRealm() = default;
						ServerCore *serverCore;
						virtual void Execute() override
						{
							serverCore->ConnectPeerToRealm(peer.get());
						}
					};
					auto com = icon7::CommandHandle<
						CommandConnectPeerToRealm>::Create();
					com->peer = peer->shared_from_this();
					com->serverCore = oldRealm->serverCore;

					if (newRealm) {
						newRealm->executionQueue.EnqueueCommand(std::move(com));
					}
				}
			}
		};
		auto com = icon7::CommandHandle<CommandDisconnectPeer>::Create();
		com->peer = peer->shared_from_this();
		com->oldRealm = oldRealm;
		com->newRealm = newRealm;
		newRealm->executionQueue.EnqueueCommand(std::move(com));
	} else {
		class CommandConnectPeerToRealm : public icon7::commands::ExecuteOnPeer
		{
		public:
			CommandConnectPeerToRealm() = default;
			~CommandConnectPeerToRealm() = default;
			std::weak_ptr<RealmServer> realm;
			virtual void Execute() override
			{
				auto r = realm.lock();
				if (r) {
					r->ConnectPeer(peer.get());
				} else {
					LOG_FATAL("Realm object already destroyed");
				}
			}
		};
		auto com = icon7::CommandHandle<CommandConnectPeerToRealm>::Create();
		com->peer = peer->shared_from_this();
		com->realm = newRealm;
		newRealm->executionQueue.EnqueueCommand(std::move(com));
	}
}

void ServerCore::RequestSpawnEntities(icon7::Peer *peer,
									  icon7::ByteReader *reader)
{
	PeerData *data = ((PeerData *)(peer->userPointer));
	std::shared_ptr<RealmServer> realm = data->realm.lock();
	if (realm && reader) {
		ClientRpcProxy::SpawnEntities_ForPeerByIds(realm.get(), peer, *reader);
	}
}

void ServerCore::InteractInLineOfSight(
	icon7::Peer *peer, ComponentLastAuthoritativeMovementState state,
	uint64_t targetId, glm::vec3 dstPos, glm::vec3 normal)
{
	// TODO: add server side verification of raycast
	PeerData *data = ((PeerData *)(peer->userPointer));
	std::shared_ptr<RealmServer> realm = data->realm.lock();
	if (realm) {
		UpdatePlayer(peer, state);
		flecs::entity entity = realm->Entity(data->entityId);
		if (entity.is_alive()) {
			auto s = entity.get<ComponentLastAuthoritativeMovementState>();
			if (s) {
				state = *s;
			}
		}
		realm->InteractInLineOfSight(peer, state, targetId, dstPos, normal);
	}
}

void ServerCore::Attack(icon7::Peer *peer,
						ComponentLastAuthoritativeMovementState state,
						uint64_t targetId, glm::vec3 targetPos,
						int64_t attackType, int64_t attackId, int64_t argInt)
{
	PeerData *data = ((PeerData *)(peer->userPointer));
	std::shared_ptr<RealmServer> realm = data->realm.lock();
	if (realm) {
		UpdatePlayer(peer, state);
		flecs::entity entity = realm->Entity(data->entityId);
		if (entity.is_alive()) {
			auto s = entity.get<ComponentLastAuthoritativeMovementState>();
			if (s) {
				state = *s;
			}
		}
		realm->Attack(peer, state, targetId, targetPos, attackType, attackId,
					  argInt);
	}
}
