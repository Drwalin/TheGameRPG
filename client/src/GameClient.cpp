#include <chrono>

#include <icon7/PeerUStcp.hpp>
#include <icon7/HostUStcp.hpp>
#include <icon7/Command.hpp>
#include <icon7/Flags.hpp>

#include "../include/ServerRpcProxy.hpp"

#include "../include/GameClient.hpp"

GameClient::GameClient() { host = nullptr; }

GameClient::~GameClient()
{
	host->DisconnectAllAsync();
	host->StopListening();
	host->WaitStopRunning();
	delete host;
	host = nullptr;
}

void GameClient::RunNetworkLoopAsync() { host->RunAsync(); }

void GameClient::DisconnectRealmPeer() {}

bool GameClient::ConnectToServer(const std::string &ip, uint16_t port)
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

void GameClient::RunOneEpoch() { realm.OneEpoch(); }
