#include "godot_cpp/variant/utility_functions.hpp"
#include "icon7/Flags.hpp"
#include "icon7/RPCEnvironment.hpp"

#include "../../../server/include/TerrainMap.hpp"
#include "../../../server/include/Entity.hpp"

#include "../include/ClientConnection.hpp"

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

godot::Array ClientConnection::GetRealms()
{
	godot::Array ar;
	for (const auto &r : realms) {
		godot::String str = godot::String::utf8(r.c_str(), r.size());
		DEBUG("adding realm to godot::Array: `%s`", r.c_str());
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
// 		rpc->Send(peer->peer.get(), icon7::FLAG_RELIABLE,
// 				  ServerRemoteFunctions::GetTerrain);
	}
}

void ClientConnection::RegisterMessages()
{
	rpc = &rpcHost->rpc;

	rpc->RegisterObjectMessage("UpdateTerrain", this,
							   &ClientConnection::UpdateTerrain,
							   &rpcHost->executionQueue);
	rpc->RegisterObjectMessage("SetRealms", this, &ClientConnection::SetRealms,
							   &rpcHost->executionQueue);
	rpc->RegisterObjectMessage("SpawnEntities", this,
							   &ClientConnection::SpawnEntities,
							   &rpcHost->executionQueue);
	rpc->RegisterObjectMessage("UpdateEntities", this,
							   &ClientConnection::UpdateEntities,
							   &rpcHost->executionQueue);
	rpc->RegisterObjectMessage("SetModel", this, &ClientConnection::SetModel,
							   &rpcHost->executionQueue);
	rpc->RegisterObjectMessage("DeleteEntities", this,
							   &ClientConnection::DeleteEntities,
							   &rpcHost->executionQueue);
	rpc->RegisterObjectMessage("SetPlayerEntityId", this,
							   &ClientConnection::SetPlayerEntityId,
							   &rpcHost->executionQueue);
}

void ClientConnection::SetRealms(std::vector<std::string> &realms)
{
	this->realms = realms;
	this->emit_signal("_ReceivedRealmsList");
}

void ClientConnection::SpawnEntities(icon7::ByteReader *reader)
{
	Entity entity;
	while (reader->has_any_more() && reader->is_valid()) {
		reader->op(entity);
		if (reader->is_valid()) {
			// TODO: update full or spawn entity
		}
	}
}

// 	{entityId, lastUpdateTick, vel, pos, forward}, ...)
void ClientConnection::UpdateEntities(icon7::ByteReader *reader)
{
	while (reader->has_any_more() && reader->is_valid()) {
		uint64_t entityId, lastUpdateTick;
		float vel[3], pos[3], forward[3];
		reader->op(entityId);
		reader->op(lastUpdateTick);
		reader->op(vel, 3);
		reader->op(pos, 3);
		reader->op(forward, 3);
		if (reader->is_valid()) {
			// TODO: update entity
		}
	}
}

void ClientConnection::SetModel(uint64_t entityId, std::string_view modelName,
								float height, float width)
{
	// TODO: set model
}

void ClientConnection::DeleteEntities(icon7::ByteReader *reader)
{
	// TODO: erase entity
}

void ClientConnection::SetPlayerEntityId(uint64_t playerEntityId)
{
	this->playerEntityId = playerEntityId;
}
