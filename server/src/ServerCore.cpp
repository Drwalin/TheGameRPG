
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
}

int ServerCore::ConnectPeerToRealm(icon7::Peer *peer, std::string realmName)
{
	if (((PeerData *)(peer->userPointer))->userName == "") {
		return 0;
	}
	auto it = realms.find(realmName);
	if (it == realms.end()) {
		return 0;
	}
	Realm *realm = it->second;
	realm->ConnectToPeer(peer);
	return 1;
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
	host = new icon7::uS::tcp::Host();
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
	while(host->IsRunningAsync()) {
		std::this_thread::sleep_for(std::chrono::seconds(1));
	}
}

void ServerCore::RunNetworkLoopAsync()
{
	host->RunAsync();
}

void ServerCore::_OnPeerConnect(icon7::Peer *peer)
{
	PeerData *data = new PeerData;
	data->peer = peer->shared_from_this();
	data->realm = nullptr;
	data->entityId = 0;
	data->userName = "";
	peer->userPointer = data;
	
	((ServerCore *)(peer->host->userPointer))->RequestRealms(peer);
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
}

void ServerCore::BindRpc()
{
	rpc.RegisterMessage("SetUsername", &ServerCore::SetUsername);
	rpc.RegisterMessage("UpdatePlayer", &ServerCore::UpdatePlayer);

	rpc.RegisterObjectMessage("GetRealms", this, &ServerCore::RequestRealms);
	rpc.RegisterObjectMessage("JoinRealm", this,
							  &ServerCore::ConnectPeerToRealm);

	rpc.RegisterMessage("GetEntitiesData", &ServerCore::RequestSpawnEntities);
	rpc.RegisterMessage("GetTerrain", &ServerCore::GetTerrain);
}

void ServerCore::RequestSpawnEntities(icon7::Peer *peer,
									  icon7::ByteReader *reader)
{
	PeerData *data = ((PeerData *)(peer->userPointer));
	Realm *realm = data->realm;
	ON_REALM_THREAD_FUNCTION_CREATOR_EXECUTION_HELPER_WITH_DATA(realm,
		peer, reader,
		{
			userData->realm->RequestSpawnEntities(peer, &reader);
		});
	
// 	if (data->realm) {
// 		icon7::commands::ExecuteOnPeer com;
// 		std::swap(com.data, reader->_data);
// 		com.peer = peer->shared_from_this();
// 		com.userPointer = (void *)(size_t)(reader->get_offset());
// 		com.function = [](icon7::Peer *peer, std::vector<uint8_t> &data,
// 						  void *ptr) {
// 			PeerData *userData = ((PeerData *)(peer->userPointer));
// 			if (userData->realm) {
// 				uint32_t offset = (uint32_t)(size_t)ptr;
// 				icon7::ByteReader reader(data, offset);
// 				userData->realm->RequestSpawnEntities(peer, &reader);
// 			}
// 		};
// 
// 		data->realm->executionQueue.EnqueueCommand(std::move(com));
// 	}
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
	ON_REALM_THREAD_FUNCTION_CREATOR_EXECUTION_HELPER_WITH_DATA(realm,
		peer, reader,
		{
			uint64_t tick;
			glm::vec3 pos;
			glm::vec3 vel;
			glm::vec3 forward;

			reader.op(tick);
			reader.op(pos);
			reader.op(vel);
			reader.op(forward);

			userData->realm->entities[userData->entityId].SolvePlayerInput(
				tick, pos, vel, forward);
		});
	
// 	if (data->realm) {
// 		icon7::commands::ExecuteOnPeer com;
// 		std::swap(com.data, reader->_data);
// 		com.peer = peer->shared_from_this();
// 		com.userPointer = (void *)(size_t)(reader->get_offset());
// 		com.function = [](icon7::Peer *peer, std::vector<uint8_t> &data,
// 						  void *ptr) {
// 			PeerData *userData = ((PeerData *)(peer->userPointer));
// 			if (userData->realm) {
// 				uint32_t offset = (uint32_t)(size_t)ptr;
// 				icon7::ByteReader reader(data, offset);
// 
// 				uint64_t tick;
// 				glm::vec3 pos;
// 				glm::vec3 vel;
// 				glm::vec3 forward;
// 
// 				reader.op(tick);
// 				reader.op(pos);
// 				reader.op(vel);
// 				reader.op(forward);
// 
// 				userData->realm->entities[userData->entityId].SolvePlayerInput(
// 					tick, pos, vel, forward);
// 			}
// 		};
// 
// 		data->realm->executionQueue.EnqueueCommand(std::move(com));
// 	}
}

void ServerCore::GetTerrain(icon7::Peer *peer)
{
	PeerData *data = ((PeerData *)(peer->userPointer));
	Realm *realm = data->realm;
	ON_REALM_THREAD_FUNCTION_CREATOR_EXECUTION_HELPER_WITHOUT_DATA(realm,
		peer,
		{
			userData->realm->rpc->Send(peer, icon7::FLAG_RELIABLE,
									   ClientRemoteFunctions::UpdateTerrain,
									   userData->realm->terrain);
		});
	
// 	if (data->realm) {
// 		icon7::commands::ExecuteOnPeer com;
// 		com.peer = peer->shared_from_this();
// 		com.function = [](icon7::Peer *peer, std::vector<uint8_t> &data,
// 						  void *ptr) {
// 			PeerData *userData = ((PeerData *)(peer->userPointer));
// 			if (userData->realm) {
// 				userData->realm->rpc->Send(peer, icon7::FLAG_RELIABLE,
// 										   ClientRemoteFunctions::UpdateTerrain,
// 										   userData->realm->terrain);
// 			}
// 		};
// 
// 		data->realm->executionQueue.EnqueueCommand(std::move(com));
// 	}
}

void ServerCore::RequestRealms(icon7::Peer *peer)
{
	rpc.Send(peer, icon7::FLAG_RELIABLE, ClientRemoteFunctions::SetRealms,
			 realmNames);
}
