#include <flecs.h>

#include <icon7/PeerUStcp.hpp>
#include <icon7/HostUStcp.hpp>
#include <icon7/Command.hpp>
#include <icon7/Flags.hpp>

#include "../../common/include/ServerRpcFunctionNames.hpp"

#include "../include/ClientRpcProxy.hpp"
#include "../include/PeerStateTransitions.hpp"

#include "../include/ServerCore.hpp"

void ServerCore::BindRpc()
{
	rpc.RegisterObjectMessage(ServerRpcFunctionNames::Login, this,
							  &ServerCore::Login);

	rpc.RegisterMessage(ServerRpcFunctionNames::UpdatePlayer,
						&ServerCore::UpdatePlayer, nullptr,
						SelectExecutionQueueByRealm);

	rpc.RegisterMessage(ServerRpcFunctionNames::GetEntitiesData,
						&ServerCore::RequestSpawnEntities, nullptr,
						SelectExecutionQueueByRealm);

	rpc.RegisterMessage(
		ServerRpcFunctionNames::Ping,
		[](icon7::Peer *peer, icon7::Flags flags, int64_t payload) {
			ClientRpcProxy::Pong(peer, flags, payload);
		},
		nullptr, nullptr);

	rpc.RegisterObjectMessage(ServerRpcFunctionNames::InteractInLineOfSight,
							  this, &ServerCore::InteractInLineOfSight, nullptr,
							  SelectExecutionQueueByRealm);
	rpc.RegisterObjectMessage(ServerRpcFunctionNames::Attack,
							  this, &ServerCore::Attack, nullptr,
							  SelectExecutionQueueByRealm);
}

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
			// TODO: verify movement state

			auto s = entity.get<ComponentLastAuthoritativeMovementState>();
			if (s && glm::length(s->oldState.pos - state.oldState.pos) < 5) {
				entity.set<ComponentLastAuthoritativeMovementState>(state);
				entity.set<ComponentMovementState>(state.oldState);
			} else {
				ClientRpcProxy::SpawnPlayerEntity_ForPlayer(realm, peer);
			}
		}
	}
}

void ServerCore::ConnectPeerToRealm(icon7::Peer *peer)
{
	LOG_INFO("TODO: replace ServerCore::ConnectPeerToRealm with something "
			 "suitable to use with database.");
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

									entity.set(ls);

									if (entity.has<ComponentMovementState>()) {
										entity.set(ls.oldState);
									}
								}

								ClientRpcProxy::SpawnPlayerEntity_ForPlayer(
									realm, peer.get());
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
		ClientRpcProxy::SpawnEntities_ForPeerByIds(realm, peer, *reader);
	}
}

void ServerCore::InteractInLineOfSight(icon7::Peer *peer,
									   ComponentMovementState state,
									   uint64_t targetId, glm::vec3 dstPos,
									   glm::vec3 normal)
{
	// TODO: add server side verification
	PeerData *data = ((PeerData *)(peer->userPointer));
	std::shared_ptr<RealmServer> realm = data->realm.lock();
	if (realm) {
		realm->InteractInLineOfSight(peer, state, targetId, dstPos, normal);
	}
}

void ServerCore::Attack(icon7::Peer *peer, ComponentMovementState state,
						uint64_t targetId, glm::vec3 targetPos,
						const std::string attackName, int64_t attackId,
						const std::string &argStr, int64_t argInt)
{
	PeerData *data = ((PeerData *)(peer->userPointer));
	std::shared_ptr<RealmServer> realm = data->realm.lock();
	if (realm) {
		realm->Attack(peer, state, targetId, targetPos, attackName, attackId,
					  argStr, argInt);
	}
}
