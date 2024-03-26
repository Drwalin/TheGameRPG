
#include <chrono>
#include <thread>

#include <icon7/PeerUStcp.hpp>
#include <icon7/HostUStcp.hpp>
#include <icon7/Command.hpp>
#include <icon7/Flags.hpp>

#include "../../common/include/ServerRpcFunctionNames.hpp"

#include "../include/ClientRpcProxy.hpp"

#include "../include/ServerCore.hpp"

void ServerCore::BindRpc()
{
	rpc.RegisterMessage(ServerRpcFunctionNames::SetUsername,
						&ServerCore::SetUsername);
	rpc.RegisterMessage(ServerRpcFunctionNames::UpdatePlayer,
						&ServerCore::UpdatePlayer, nullptr,
						SelectExecutionQueue);

	rpc.RegisterObjectMessage(ServerRpcFunctionNames::GetRealms, this,
							  &ServerCore::RequestRealms);
	rpc.RegisterObjectMessage(ServerRpcFunctionNames::JoinRealm, this,
							  &ServerCore::ConnectPeerToRealm, nullptr,
							  SelectExecutionQueueForJoinRealm);
	rpc.RegisterObjectMessage(ServerRpcFunctionNames::GetCurrentTick, this,
							  &ServerCore::GetCurrentTick);

	rpc.RegisterMessage(ServerRpcFunctionNames::GetEntitiesData,
						&ServerCore::RequestSpawnEntities, nullptr,
						SelectExecutionQueue);

	rpc.RegisterMessage(
		ServerRpcFunctionNames::Ping,
		[](icon7::Peer *peer, icon7::Flags flags, uint64_t payload) {
			ClientRpcProxy::Pong(peer, payload);
		},
		nullptr, nullptr);
}

icon7::CommandExecutionQueue *
ServerCore::SelectExecutionQueue(icon7::MessageConverter *messageConverter,
								 icon7::Peer *peer, icon7::ByteReader &reader,
								 icon7::Flags flags)
{
	PeerData *data = ((PeerData *)(peer->userPointer));
	RealmServer *realm = data->realm;
	if (realm) {
		return &realm->executionQueue;
	}
	return nullptr;
}

icon7::CommandExecutionQueue *ServerCore::SelectExecutionQueueForJoinRealm(
	icon7::MessageConverter *messageConverter, icon7::Peer *peer,
	icon7::ByteReader &reader, icon7::Flags flags)
{
	bitscpp::ByteReader<true> r2(reader);
	std::string realmName;
	r2.op(realmName);

	auto core = ((ServerCore *)(peer->host->userPointer));

	RealmServer *newRealm = core->realmManager.GetRealm(realmName);
	if (newRealm) {
		DEBUG("Choosing queue for join realm: %p", &newRealm->executionQueue);
		return &newRealm->executionQueue;
	} else {
		DEBUG("Invalid realm name");
		return nullptr;
	}
	DEBUG("Choosing queue for join realm: null");
	return nullptr;
}

void ServerCore::SetUsername(icon7::Peer *peer, std::string_view userName)
{
	// TODO: add login with password verification
	((PeerData *)(peer->userPointer))->userName = userName;
}

void ServerCore::UpdatePlayer(icon7::Peer *peer,
							  const EntityLastAuthoritativeMovementState &state)
{
	PeerData *data = ((PeerData *)(peer->userPointer));
	RealmServer *realm = data->realm;
	if (realm) {
		flecs::entity entity = realm->Entity(data->entityId);
		if (entity.is_alive()) {
			entity.set<EntityLastAuthoritativeMovementState>(state);
		}
	}
}

void ServerCore::RequestRealms(icon7::Peer *peer)
{
	std::vector<std::string> realmNames;
	realmManager.GetRealmNames(realmNames);
	ClientRpcProxy::SetRealms(peer, realmNames);
}

void ServerCore::ConnectPeerToRealm(icon7::Peer *peer, std::string realmName)
{
	PeerData *data = ((PeerData *)(peer->userPointer));
	if (data->userName == "") {
		DEBUG("Invalid usernamne");
		return;
	}
	RealmServer *newRealm = realmManager.GetRealm(realmName);
	if (newRealm == nullptr) {
		DEBUG("Invalid realm");
		return;
	}

	RealmServer *oldRealm = data->realm;
	if (oldRealm) {
		oldRealm->DisconnectPeer(peer);

		DEBUG("Connecting peer to realm stage 1");

		icon7::commands::ExecuteOnPeer com;
		com.data.resize(realmName.size() + 1);
		memcpy(com.data.data(), realmName.data(), com.data.size());
		com.peer = peer->shared_from_this();
		com.userPointer = this;
		com.function = [](icon7::Peer *peer, std::vector<uint8_t> &fname,
						  void *ptr) {
			((ServerCore *)ptr)
				->ConnectPeerToRealm(peer,
									 {(char *)fname.data(), fname.size() - 1});
		};

		newRealm->executionQueue.EnqueueCommand(std::move(com));
	} else {
		DEBUG("Connecting peer to realm, stage 2");

		icon7::commands::ExecuteOnPeer com;
		com.peer = peer->shared_from_this();
		com.userPointer = newRealm;
		com.function = [](icon7::Peer *peer, std::vector<uint8_t> &fname,
						  void *ptr) {
			DEBUG("Connecting peer to realm, stage final");
			RealmServer *realm = ((RealmServer *)ptr);
			realm->ConnectPeer(peer);
		};

		newRealm->executionQueue.EnqueueCommand(std::move(com));
	}
}

void ServerCore::GetCurrentTick(icon7::Peer *peer, icon7::Flags flags)
{
	PeerData *data = ((PeerData *)(peer->userPointer));
	RealmServer *realm = data->realm;
	if (realm) {
		ClientRpcProxy::SetCurrentTick(realm, peer);
	}
}

void ServerCore::RequestSpawnEntities(icon7::Peer *peer,
									  icon7::ByteReader *reader)
{
	PeerData *data = ((PeerData *)(peer->userPointer));
	RealmServer *realm = data->realm;
	if (realm && reader) {
		ClientRpcProxy::SpawnEntities_ForPeerByIds(realm, peer, *reader);
	}
}
