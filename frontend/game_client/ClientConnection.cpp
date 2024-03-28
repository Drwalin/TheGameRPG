#include "godot_cpp/classes/resource_preloader.hpp"
#include "godot_cpp/variant/utility_functions.hpp"

#include "icon7/Flags.hpp"
#include "icon7/RPCEnvironment.hpp"

#include "../include/ClientConnection.hpp"

ClientConnection::ClientConnection() : entities(this)
{
}

void ClientConnection::_bind_methods()
{
	METHOD_NO_ARGS(ClientConnection, GetRealms);
	METHOD_NO_ARGS(ClientConnection, RefreshRealms);
	METHOD_ARGS(ClientConnection, SetUsername, "username");
	METHOD_ARGS(ClientConnection, EnterRealm, "realmName");

	METHOD_ARGS(ClientConnection, Connect, "address", "port");
	METHOD_ARGS(ClientConnection, _OnConnected, "client");

	METHOD_ARGS(ClientConnection, SetClient, "client");
	METHOD_NO_ARGS(ClientConnection, GetClient);
	godot::ClassDB::add_property(
		"ClientConnection",
		godot::PropertyInfo(godot::Variant::OBJECT, "client"), "SetClient",
		"GetClient");

	METHOD_ARGS(ClientConnection, SetHost, "host");
	METHOD_NO_ARGS(ClientConnection, GetHost);
	godot::ClassDB::add_property(
		"ClientConnection", godot::PropertyInfo(godot::Variant::OBJECT, "host"),
		"SetHost", "GetHost");
	
	godot::ClassDB::add_signal("ClientConnection", godot::MethodInfo("_ReceivedRealmsList"));
}

void ClientConnection::Connect(const godot::String &address, int64_t port)
{
	rpcHost->Connect(address, port, godot::Callable(this, "_OnConnected"));
}

void ClientConnection::_OnConnected(RpcClient *client)
{
	if (client) {
		if (peer) {
			peer->Disconnect();
		}

		SetClient(client);
		RefreshRealms();
	}
}

RpcClient *ClientConnection::GetClient() { return peer; }

void ClientConnection::SetClient(RpcClient *peer)
{
	this->peer = peer;
}

RpcHost *ClientConnection::GetHost() { return rpcHost; }

void ClientConnection::SetHost(RpcHost *rpcHost) { this->rpcHost = rpcHost; }

void ClientConnection::_enter_tree()
{
	rpcHost = this->get_node<RpcHost>("/root/rpcHost");
	if (rpcHost) {
		RegisterMessages();
	}
}

void ClientConnection::_process(double dt)
{
	entities.Update();
}

godot::Array ClientConnection::GetRealms()
{
	godot::Array ar;
	for (const auto &r : realms) {
		godot::String str = godot::String::utf8(r.c_str(), r.size());
		ar.append(str);
	}
	return ar;
}

void ClientConnection::RefreshRealms()
{
	if (peer && rpc) {
		rpc->Send(peer->peer.get(), icon7::FLAG_RELIABLE,
				  ServerRemoteFunctions::GetRealms);
	}
}

void ClientConnection::SetUsername(const godot::String &userName)
{
	if (peer) {
		std::string str = userName.utf8().ptr();
		rpc->Send(peer->peer.get(), icon7::FLAG_RELIABLE,
				  ServerRemoteFunctions::SetUsername, str);
		username = str;
	}
}

void ClientConnection::EnterRealm(const godot::String &realmName)
{
	if (peer) {
		std::string str = realmName.utf8().ptr();
		rpc->Send(peer->peer.get(), icon7::FLAG_RELIABLE,
				  ServerRemoteFunctions::JoinRealm, str);
	}
}

void ClientConnection::RequestEntityData(uint64_t entityId)
{
	if (peer) {
		rpc->Send(peer->peer.get(), icon7::FLAG_RELIABLE|icon7::FLAGS_CALL_NO_FEEDBACK, ServerRemoteFunctions::GetEntitiesData, entityId);
	}
}
