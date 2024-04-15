#include <icon7/PeerUStcp.hpp>
#include <icon7/HostUStcp.hpp>
#include <icon7/Command.hpp>
#include <icon7/Flags.hpp>

#include "../../common/include/ServerRpcFunctionNames.hpp"

#include "../include/ClientRpcProxy.hpp"

#include "../include/ServerCore.hpp"

void ServerCore::BindRpc()
{
	rpc.RegisterMessage(ServerRpcFunctionNames::Login, &ServerCore::Login);
	rpc.RegisterMessage(ServerRpcFunctionNames::UpdatePlayer,
						&ServerCore::UpdatePlayer, nullptr,
						SelectExecutionQueue);

	rpc.RegisterMessage(ServerRpcFunctionNames::GetEntitiesData,
						&ServerCore::RequestSpawnEntities, nullptr,
						SelectExecutionQueue);

	rpc.RegisterMessage(
		ServerRpcFunctionNames::Ping,
		[](icon7::Peer *peer, icon7::Flags flags, uint64_t payload) {
			ClientRpcProxy::Pong(peer, flags, payload);
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

void ServerCore::Login(icon7::Peer *peer, const std::string &userName,
					   const std::string &password)
{
	// TODO: add login with password verification
	((PeerData *)(peer->userPointer))->userName = userName;
	ClientRpcProxy::LoginSuccessfull(peer);
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

void ServerCore::ConnectPeerToRealm(icon7::Peer *peer, std::string realmName)
{
	PeerData *data = ((PeerData *)(peer->userPointer));
	if (data->userName == "") {
		DEBUG("Invalid usernamne");
// 		return;
	}
	RealmServer *newRealm = realmManager.GetRealm(realmName);
	if (newRealm == nullptr) {
		DEBUG("Invalid realm");
		return;
	}

	RealmServer *oldRealm = data->realm;
	if (oldRealm) {
		oldRealm->DisconnectPeer(peer);

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
		icon7::commands::ExecuteOnPeer com;
		com.peer = peer->shared_from_this();
		com.userPointer = newRealm;
		com.function = [](icon7::Peer *peer, std::vector<uint8_t> &fname,
						  void *ptr) {
			RealmServer *realm = ((RealmServer *)ptr);
			realm->ConnectPeer(peer);
		};

		newRealm->executionQueue.EnqueueCommand(std::move(com));
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
