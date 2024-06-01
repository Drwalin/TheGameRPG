#include <icon7/PeerUStcp.hpp>
#include <icon7/HostUStcp.hpp>
#include <icon7/Command.hpp>
#include <icon7/Flags.hpp>

#include "../../common/include/ServerRpcFunctionNames.hpp"
#include "../../common/include/ClientRpcFunctionNames.hpp"

#include "../include/ClientRpcProxy.hpp"
#include "../include/PeerStateTransitions.hpp"

#include "../include/ServerCore.hpp"

void ServerCore::BindRpc()
{
	rpc.RegisterMessage(ServerRpcFunctionNames::Login, &ServerCore::Login);
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
		peer_transitions::OnReceivedLogin(peer, userName);
	} else {
		LOG_ERROR("Peer received Login request but PeerData is NULL");
	}
}

void ServerCore::UpdatePlayer(icon7::Peer *peer,
							  const EntityLastAuthoritativeMovementState &state)
{
	PeerData *data = ((PeerData *)(peer->userPointer));
	std::shared_ptr<RealmServer> realm = data->realm.lock();
	if (realm) {
		flecs::entity entity = realm->Entity(data->entityId);
		if (entity.is_alive()) {
			entity.set<EntityLastAuthoritativeMovementState>(state);
			entity.set<EntityMovementState>(state.oldState);

			// TODO: verify movement state
			realm->BroadcastUnreliableExcept(
				data->entityId, ClientRpcFunctionNames::UpdateEntities,
				data->entityId, state);
		}
	}
}

void ServerCore::ConnectPeerToRealm(icon7::Peer *peer, std::string realmName)
{
	LOG_INFO("TODO: replace ServerCore::ConnectPeerToRealm with something "
			 "suitable to use with database.");
	PeerData *data = ((PeerData *)(peer->userPointer));
	if (data->userName == "") {
		LOG_INFO("Invalid usernamne");
		// 		return;
	}
	std::shared_ptr<RealmServer> newRealm = realmManager.GetRealm(realmName);
	if (newRealm == nullptr) {
		LOG_INFO("Invalid realm");
		return;
	}

	std::shared_ptr<RealmServer> oldRealm = data->realm.lock();
	if (oldRealm) {
		oldRealm->DisconnectPeer(peer);

		class CommandConnectPeerToRealm : public icon7::commands::ExecuteOnPeer
		{
		public:
			CommandConnectPeerToRealm() = default;
			~CommandConnectPeerToRealm() = default;
			std::string realmName;
			ServerCore *serverCore;
			virtual void Execute() override
			{
				serverCore->ConnectPeerToRealm(peer.get(), realmName);
			}
		};
		auto com = icon7::CommandHandle<CommandConnectPeerToRealm>::Create();
		com->realmName = realmName;
		com->peer = peer->shared_from_this();
		com->serverCore = this;
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
