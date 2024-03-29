#include <chrono>
#include <thread>

#include <icon7/PeerUStcp.hpp>
#include <icon7/HostUStcp.hpp>
#include <icon7/Command.hpp>
#include <icon7/Flags.hpp>

#include "../include/ServerRpcProxy.hpp"

#include "../include/ClientCore.hpp"

ClientCore::ClientCore() { host = nullptr; }

ClientCore::~ClientCore()
{
	host->DisconnectAllAsync();
	host->StopListening();
	host->WaitStopRunning();
	delete host;
	host = nullptr;
}

void ClientCore::RunNetworkLoopSync()
{
	RunNetworkLoopAsync();
	std::this_thread::sleep_for(std::chrono::seconds(10));
	while (host->IsRunningAsync()) {
		std::this_thread::sleep_for(std::chrono::seconds(1));
	}
}

void ClientCore::RunNetworkLoopAsync() { host->RunAsync(); }

void ClientCore::DisconnectRealmPeer() {}

bool ClientCore::ConnectToRealmServer(const std::string &ip, uint16_t port)
{
	DisconnectRealmPeer();
	
	std::future<std::shared_ptr<icon7::Peer>> peerFuture = host->ConnectPromise(ip, port);
	peerFuture.wait_for(std::chrono::seconds(5));
	if (peerFuture.valid() == false) {
		return false;
	}
	realmConnectionPeer = peerFuture.get();
	
	ServerRpcProxy::SetUsername(this, username);
	ServerRpcProxy::GetCurrentTick(this);
	
	return true;
}

void ClientCore::RunOneEpoch() { realm.OneEpoch(); }
