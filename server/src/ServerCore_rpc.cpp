
#include <chrono>
#include <thread>

#include <icon7/PeerUStcp.hpp>
#include <icon7/HostUStcp.hpp>
#include <icon7/Command.hpp>
#include <icon7/Flags.hpp>

#include "../include/ServerCore.hpp"
#include "../include/Realm.hpp"

void ServerCore::BindRpc()
{
	rpc.RegisterMessage("SetUsername", &ServerCore::SetUsername);
	rpc.RegisterMessage("UpdatePlayer", &ServerCore::UpdatePlayer, nullptr,
						SelectExecutionQueue);

	rpc.RegisterObjectMessage("GetRealms", this, &ServerCore::RequestRealms);
	rpc.RegisterObjectMessage("JoinRealm", this,
							  &ServerCore::ConnectPeerToRealm, nullptr,
							  SelectExecutionQueueForJoinRealm);
	rpc.RegisterObjectMessage("GetCurrentTick", this,
							  &ServerCore::GetCurrentTick);

	rpc.RegisterMessage("GetEntitiesData", &ServerCore::RequestSpawnEntities,
						nullptr, SelectExecutionQueue);
	rpc.RegisterMessage("GetTerrain", &ServerCore::GetTerrain, nullptr,
						SelectExecutionQueue);

	rpc.RegisterMessage(
		"Ping",
		[](icon7::Peer *peer, icon7::Flags flags, uint64_t payload) {
			peer->host->GetRpcEnvironment()->Send(
				peer, flags, ClientRemoteFunctions::Pong, payload);
		},
		nullptr, nullptr);
}

icon7::CommandExecutionQueue *
ServerCore::SelectExecutionQueue(icon7::MessageConverter *messageConverter,
								 icon7::Peer *peer, icon7::ByteReader &reader,
								 icon7::Flags flags)
{
	PeerData *data = ((PeerData *)(peer->userPointer));
	Realm *realm = data->realm;
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

	auto it = core->realms.find((std::string)realmName);
	if (it == core->realms.end()) {
		DEBUG("Invalid realm");
		return nullptr;
	}
	Realm *newRealm = it->second;
	if (newRealm) {
		DEBUG("Choosing queue for join realm: %p", &newRealm->executionQueue);
		return &newRealm->executionQueue;
	}
	DEBUG("Choosing queue for join realm: null");
	return nullptr;
}

void ServerCore::SetUsername(icon7::Peer *peer, std::string_view userName)
{
	// TODO: add login with password verification
	((PeerData *)(peer->userPointer))->userName = userName;
}

void ServerCore::UpdatePlayer(icon7::Peer *peer, icon7::ByteReader *reader)
{
	PeerData *data = ((PeerData *)(peer->userPointer));
	Realm *realm = data->realm;
	if (realm) {
		uint64_t tick;
		glm::vec3 pos;
		glm::vec3 vel;
		glm::vec3 forward;

		reader->op(tick);
		reader->op(pos);
		reader->op(vel);
		reader->op(forward);

		realm->entities[data->entityId].SolvePlayerInput(tick, pos, vel,
														 forward);
	}
}

void ServerCore::RequestRealms(icon7::Peer *peer)
{
	rpc.Send(peer, icon7::FLAG_RELIABLE, ClientRemoteFunctions::SetRealms,
			 realmNames);
}

void ServerCore::ConnectPeerToRealm(icon7::Peer *peer,
									std::string_view realmName)
{
	PeerData *data = ((PeerData *)(peer->userPointer));
	if (data->userName == "") {
		DEBUG("Invalid usernamne");
		return;
	}
	auto it = realms.find((std::string)realmName);
	if (it == realms.end()) {
		DEBUG("Invalid realm");
		return;
	}
	Realm *newRealm = it->second;

	Realm *oldRealm = data->realm;
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
			((Realm *)ptr)->ConnectToPeer(peer);
		};

		newRealm->executionQueue.EnqueueCommand(std::move(com));
	}
}

void ServerCore::GetCurrentTick(icon7::Peer *peer, icon7::Flags flags)
{
	PeerData *data = ((PeerData *)(peer->userPointer));
	Realm *realm = data->realm;
	if (realm) {
		uint64_t tick = realm->timer.CalcCurrentTick();
		;
		rpc.Send(peer, flags, ClientRemoteFunctions::SetCurrentTick, tick);
	}
}

void ServerCore::RequestSpawnEntities(icon7::Peer *peer,
									  icon7::ByteReader *reader)
{
	PeerData *data = ((PeerData *)(peer->userPointer));
	Realm *realm = data->realm;
	if (realm) {
		realm->RequestSpawnEntities(peer, reader);
	}
}

void ServerCore::GetTerrain(icon7::Peer *peer)
{
	PeerData *data = ((PeerData *)(peer->userPointer));
	Realm *realm = data->realm;
	if (realm) {
		DEBUG("Sending terrain");
		realm->rpc->Send(peer, icon7::FLAG_RELIABLE,
						 ClientRemoteFunctions::UpdateTerrain, realm->terrain);
	} else {
		DEBUG("Not sending terrain");
	}
}
