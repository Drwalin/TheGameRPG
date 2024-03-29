
#include <chrono>
#include <thread>

#include <icon7/PeerUStcp.hpp>
#include <icon7/HostUStcp.hpp>
#include <icon7/Command.hpp>
#include <icon7/Flags.hpp>

#include "../include/ServerCore.hpp"

ServerCore::ServerCore() { host = nullptr; }

ServerCore::~ServerCore()
{
	host->DisconnectAllAsync();
	host->StopListening();
	host->WaitStopRunning();
	delete host;
	host = nullptr;
}

void ServerCore::CreateRealm(std::string realmName)
{
	RealmServer *realm = new RealmServer();
	realm->serverCore = this;
	realm->rpc = &rpc;
	realm->Init(realmName);
	realmManager.AddNewRealm(realm);
	spawnRealm = realmName;
}

void ServerCore::Disconnect(icon7::Peer *peer)
{
	RealmServer *realm = ((PeerData *)(peer->userPointer))->realm;
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

	auto core = ((ServerCore *)(peer->host->userPointer));
	// TODO: get player data from database and call core->ConnectPeerToRealm
	core->ConnectPeerToRealm(peer, core->spawnRealm);
}

void ServerCore::_OnPeerDisconnect(icon7::Peer *peer)
{
	PeerData *data = ((PeerData *)(peer->userPointer));
	if (data->realm) {
		std::vector<uint8_t> v;
		data->realm->ExecuteOnRealmThread(peer, v,
				[](icon7::Peer *peer, std::vector<uint8_t> &, void *realm)
				{
					((RealmServer *)realm)->DisconnectPeer(peer);
				});
		data->peer = nullptr;
		data->userName = "";
	}
	delete data;
	peer->userPointer = nullptr;
}
