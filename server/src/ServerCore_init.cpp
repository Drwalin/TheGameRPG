#include <flecs.h>

#include <icon7/RPCEnvironment.hpp>
#include <icon7/Flags.hpp>
#include <icon7/PeerUStcp.hpp>
#include <icon7/HostUStcp.hpp>
#include <icon7/LoopUS.hpp>

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

	if (loop) {
		host = nullptr;
		loop->Destroy();
		loop = nullptr;
	}
}

void ServerCore::StartService()
{
	std::shared_ptr<icon7::uS::Loop> loop = std::make_shared<icon7::uS::Loop>();
	this->loop = loop;
	loop->Init(3);
	loop->userPointer = this;
	
	std::shared_ptr<icon7::uS::tcp::Host> host = loop->CreateHost(false);
	this->host = host;
	host->userPointer = this;

	host->SetOnConnect(_OnPeerConnect);
	host->SetOnDisconnect(_OnPeerDisconnect);

	host->SetRpcEnvironment(rpc);
}
