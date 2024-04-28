
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

void ServerCore::StartService()
{
	icon7::uS::tcp::Host *_host = new icon7::uS::tcp::Host();
	_host->Init();
	host = _host;
	host->userPointer = this;

	host->SetOnConnect(_OnPeerConnect);
	host->SetOnDisconnect(_OnPeerDisconnect);

	host->SetRpcEnvironment(&rpc);
}

void ServerCore::Listen(const std::string &addressInterface, uint16_t port,
						int useIpv4)
{
	host->ListenOnPort(addressInterface, port,
					   useIpv4 ? icon7::IPv4 : icon7::IPv6);
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
	LOG_TRACE("DISCONNECTING PEER on network thread enter");
	PeerData *data = ((PeerData *)(peer->userPointer));
	if (data->realm) {

		class CommandExecutePeerDisconnectOnRealm
			: public icon7::commands::ExecuteOnPeer
		{
		public:
			CommandExecutePeerDisconnectOnRealm() = default;
			~CommandExecutePeerDisconnectOnRealm() = default;
			RealmServer *realmServer;
			virtual void Execute() override
			{
				realmServer->DisconnectPeer(peer.get());
				PeerData *data = ((PeerData *)(peer->userPointer));
				data->peer = nullptr;
				data->userName = "";
				delete data;
				peer->userPointer = nullptr;
				LOG_TRACE("DISCONNECTING PEER on realm's thread");
			}
		};
		auto com =
			icon7::CommandHandle<CommandExecutePeerDisconnectOnRealm>::Create();
		com->peer = peer->shared_from_this();
		com->realmServer = data->realm;
		data->realm->ExecuteOnRealmThread(std::move(com));
	} else {
		delete data;
		peer->userPointer = nullptr;
		LOG_TRACE("DISCONNECTING PEER on network thread");
	}
}
