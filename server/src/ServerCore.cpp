#include <icon7/Command.hpp>
#include <icon7/Debug.hpp>
#include <icon7/Peer.hpp>
#include <icon7/Host.hpp>
#include <icon7/Flags.hpp>

#include "../include/FunctorCommands.hpp"

#include "../include/ServerCore.hpp"

void ServerCore::CreateRealm(std::string realmName)
{
	std::shared_ptr<RealmServer> realm = std::make_shared<RealmServer>();
	realm->serverCore = this;
	realm->rpc = rpc;
	realm->Init(realmName);
	realmManager.AddNewRealm(realm);
	spawnRealm = realmName;
}

void ServerCore::Disconnect(icon7::Peer *peer)
{
	PeerData *data = ((PeerData *)(peer->userPointer));
	std::shared_ptr<RealmServer> realm = data->realm.lock();
	if (realm) {
		realm->DisconnectPeer(peer);
	}
}

void ServerCore::Listen(const std::string &addressInterface, uint16_t port,
						int useIpv4)
{
	class CommandExecuteListeningResult
		: public icon7::commands::ExecuteBooleanOnHost
	{
	public:
		CommandExecuteListeningResult() = default;
		~CommandExecuteListeningResult() = default;
		ServerCore *serverCore;
		std::string address;
		uint16_t port;
		bool ipv4;
		virtual void Execute() override
		{
			if (result) {
				LOG_INFO("Listening on %s[:%i] ipv%i", address.c_str(),
						 (int)port, ipv4 ? 4 : 6);
			} else {
				LOG_INFO("Failed to listen on %s[:%i] ipv%i", address.c_str(),
						 (int)port, ipv4 ? 4 : 6);
			}
		}
	};
	auto com = icon7::CommandHandle<CommandExecuteListeningResult>::Create();
	com->serverCore = this;
	com->address = addressInterface;
	com->port = port;
	com->ipv4 = useIpv4;
	host->ListenOnPort(addressInterface, port,
					   useIpv4 ? icon7::IPv4 : icon7::IPv6, std::move(com));
}

void ServerCore::RunNetworkLoopAsync()
{
	host->RunAsync();
	host->EnqueueCommand(
		icon7::CommandHandle<CommandFunctor<void (*)()>>::Create(
			+[]() { LOG_INFO("Networking thread started"); }));
}

void ServerCore::_OnPeerConnect(icon7::Peer *peer)
{
	PeerData *data = new PeerData;
	data->peer = peer->weak_from_this();
	data->realm.reset();
	data->entityId = 0;
	data->userName = "";
	data->peerState = WAITING_FOR_USERNAME;
	peer->userPointer = data;
}

void ServerCore::_OnPeerDisconnect(icon7::Peer *peer)
{
	PeerData *data = ((PeerData *)(peer->userPointer));
	auto realm = data->realm.lock();
	if (realm) {

		class CommandExecutePeerDisconnectOnRealm
			: public icon7::commands::ExecuteOnPeer
		{
		public:
			CommandExecutePeerDisconnectOnRealm() = default;
			~CommandExecutePeerDisconnectOnRealm() = default;
			std::weak_ptr<RealmServer> realmServer;
			virtual void Execute() override
			{
				auto r = realmServer.lock();
				if (r) {
					r->DisconnectPeer(peer.get());
					PeerData *data = ((PeerData *)(peer->userPointer));
					data->peer.reset();
					r->serverCore->usernameToPeer.erase(data->userName);
					data->userName = "";
					delete data;
					peer->userPointer = nullptr;
				} else {
					LOG_FATAL("Realm object already destroyed");
				}
			}
		};
		auto com =
			icon7::CommandHandle<CommandExecutePeerDisconnectOnRealm>::Create();
		com->peer = peer->shared_from_this();
		com->realmServer = realm;
		realm->ExecuteOnRealmThread(std::move(com));
	} else {
		delete data;
		peer->userPointer = nullptr;
	}
}
