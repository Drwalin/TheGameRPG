
#include <chrono>
#include <thread>

#include <icon7/PeerUStcp.hpp>
#include <icon7/HostUStcp.hpp>
#include <icon7/Command.hpp>
#include <icon7/Flags.hpp>

#include "../include/ServerCore.hpp"
#include "../include/Realm.hpp"

ServerCore::ServerCore() { host = nullptr; }

ServerCore::~ServerCore()
{
	host->DisconnectAllAsync();
	host->StopListening();
	host->WaitStopRunning();
	delete host;
	host = nullptr;
	for (auto it : realms) {
		delete it.second;
	}
}

void ServerCore::CreateRealm(std::string realmName)
{
	Realm *realm = new Realm();
	realm->realmName = realmName;
	realm->Init();
	// TODO: add map entities to Realm
	realms[realmName] = realm;
	realmNames.push_back(realmName);
	realm->serverCore = this;
	realm->rpc = &rpc;
	realm->RunAsync();
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

void ServerCore::Disconnect(icon7::Peer *peer)
{
	Realm *realm = ((PeerData *)(peer->userPointer))->realm;
	if (realm) {
		realm->DisconnectPeer(peer);
	}
}

void ServerCore::StartListening(uint16_t port, int useIpv4)
{
	icon7::uS::tcp::Host *_host = new icon7::uS::tcp::Host();
	_host->Init();
	host = _host;
	host->userPointer = this;

	host->SetOnConnect(_OnPeerConnect);
	host->SetOnDisconnect(_OnPeerDisconnect);

	host->SetRpcEnvironment(&rpc);
	host->ListenOnPort(port, useIpv4 ? icon7::IPv4 : icon7::IPv6);
}

void ServerCore::RunNetworkLoopSync()
{
	RunNetworkLoopAsync();
	std::this_thread::sleep_for(std::chrono::seconds(10));
	while (host->IsRunningAsync()) {
		std::this_thread::sleep_for(std::chrono::seconds(1));
	}
}

void ServerCore::RunNetworkLoopAsync() { host->RunAsync(); }

void ServerCore::_OnPeerConnect(icon7::Peer *peer)
{
	PeerData *data = new PeerData;
	data->peer = peer->shared_from_this();
	data->realm = nullptr;
	data->entityId = 0;
	data->userName = "";
	peer->userPointer = data;
	
	DEBUG("New peer connected: %p", peer);

	((ServerCore *)(peer->host->userPointer))->RequestRealms(peer);
	
	int count = 0;
	peer->host->ForEachPeer([&](auto p)
			{
				count++;
			});
	DEBUG("Total peers count on connect: %i", count);
}

void ServerCore::_OnPeerDisconnect(icon7::Peer *peer)
{
	PeerData *data = ((PeerData *)(peer->userPointer));
	if (data->realm) {
		data->realm->DisconnectPeer(peer);
		data->peer = nullptr;
		data->userName = "";
	}
	delete data;
	peer->userPointer = nullptr;
	
	int count = 0;
	peer->host->ForEachPeer([&](auto p)
			{
				count++;
			});
	DEBUG("Total peers count on disconnect: %i", count);
}

void ServerCore::BindRpc()
{
	rpc.RegisterMessage("SetUsername", &ServerCore::SetUsername);
	rpc.RegisterMessage("UpdatePlayer", &ServerCore::UpdatePlayer, nullptr,
						SelectExecutionQueue);

	rpc.RegisterObjectMessage("GetRealms", this, &ServerCore::RequestRealms);
	rpc.RegisterObjectMessage("JoinRealm", this,
							  &ServerCore::ConnectPeerToRealm, nullptr,
							  SelectExecutionQueueForJoinRealm);

	rpc.RegisterMessage("GetEntitiesData", &ServerCore::RequestSpawnEntities,
						nullptr, SelectExecutionQueue);
	rpc.RegisterMessage("GetTerrain", &ServerCore::GetTerrain, nullptr,
						SelectExecutionQueue);
}

icon7::CommandExecutionQueue *
ServerCore::SelectExecutionQueue(icon7::MessageConverter *messageConverter,
					 icon7::Peer *peer, icon7::ByteReader &reader,
					 icon7::Flags flags)
{
	PeerData *data = ((PeerData *)(peer->userPointer));
	if (data->realm) {
		return &data->realm->executionQueue;
	}
	return nullptr;
}

icon7::CommandExecutionQueue *
ServerCore::SelectExecutionQueueForJoinRealm(icon7::MessageConverter *messageConverter,
					 icon7::Peer *peer, icon7::ByteReader &reader,
					 icon7::Flags flags)
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

void ServerCore::RequestSpawnEntities(icon7::Peer *peer,
									  icon7::ByteReader *reader)
{
	PeerData *data = ((PeerData *)(peer->userPointer));
	if (data->realm) {
		data->realm->RequestSpawnEntities(peer, reader);
	}
}

void ServerCore::SetUsername(icon7::Peer *peer, std::string_view userName)
{
	// TODO: add login with password verification
	((PeerData *)(peer->userPointer))->userName = userName;
}

void ServerCore::UpdatePlayer(icon7::Peer *peer, icon7::ByteReader *reader)
{
	PeerData *data = ((PeerData *)(peer->userPointer));
	if (data->realm) {
		uint64_t tick;
		glm::vec3 pos;
		glm::vec3 vel;
		glm::vec3 forward;

		reader->op(tick);
		reader->op(pos);
		reader->op(vel);
		reader->op(forward);

		data->realm->entities[data->entityId].SolvePlayerInput(tick, pos, vel,
															   forward);
	}
}

void ServerCore::GetTerrain(icon7::Peer *peer)
{
	PeerData *data = ((PeerData *)(peer->userPointer));
	if (data->realm) {
		DEBUG("Sending terrain");
		data->realm->rpc->Send(peer, icon7::FLAG_RELIABLE,
							   ClientRemoteFunctions::UpdateTerrain,
							   data->realm->terrain);
	} else {
		DEBUG("Not sending terrain");
	}
}

void ServerCore::RequestRealms(icon7::Peer *peer)
{
	rpc.Send(peer, icon7::FLAG_RELIABLE, ClientRemoteFunctions::SetRealms,
			 realmNames);
}
