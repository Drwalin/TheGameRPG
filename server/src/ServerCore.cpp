
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
	peer->host->ForEachPeer([&](auto p) { count++; });
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
	peer->host->ForEachPeer([&](auto p) { count++; });
	DEBUG("Total peers count on disconnect: %i", count);
}
