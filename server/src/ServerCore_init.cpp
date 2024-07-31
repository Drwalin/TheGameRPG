#include <flecs.h>

#include <icon7/RPCEnvironment.hpp>
#include <icon7/Flags.hpp>
#include <icon7/PeerUStcp.hpp>
#include <icon7/HostUStcp.hpp>

#include "../include/ServerCore.hpp"

ServerCore::ServerCore() : commandParser(this), configStorage(this)
{
	host = nullptr;
	rpc = new icon7::RPCEnvironment();
}

ServerCore::~ServerCore()
{
	Destroy();
	delete rpc;
	rpc = nullptr;
}

void ServerCore::Destroy()
{
	if (host) {
		host->DisconnectAllAsync();
		host->StopListening();
		host->DisconnectAllAsync();
	}

	realmManager.DestroyAllRealmsAndStop();

	if (host) {
		host->WaitStopRunning();
		host->_InternalDestroy();
		delete host;
		host = nullptr;
	}
}

void ServerCore::StartService()
{
	icon7::uS::tcp::Host *_host = new icon7::uS::tcp::Host();
	_host->Init();
	host = _host;
	host->userPointer = this;

	host->SetOnConnect(_OnPeerConnect);
	host->SetOnDisconnect(_OnPeerDisconnect);

	host->SetRpcEnvironment(rpc);
}
