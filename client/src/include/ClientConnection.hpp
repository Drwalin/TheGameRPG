#pragma once

#include "godot_cpp/variant/utility_functions.hpp"
#include <godot_cpp/godot.hpp>
#include <godot_cpp/core/class_db.hpp>

#include <gdextension_interface.h>
#include <godot_cpp/core/defs.hpp>
#include <godot_cpp/godot.hpp>
#include <godot_cpp/core/engine_ptrcall.hpp>
#include <godot_cpp/classes/scene_tree.hpp>

#include <godot_cpp/classes/node2d.hpp>
#include <godot_cpp/variant/packed_byte_array.hpp>
#include <godot_cpp/classes/ref_counted.hpp>

#include "../../../ICon7-godot-client/include/icon7-godot-client/Connections.hpp"

#define METHOD_NO_ARGS(CLASS, NAME)                                            \
	godot::ClassDB::bind_method(godot::D_METHOD(#NAME), &CLASS::NAME);

#define METHOD_ARGS(CLASS, NAME, ...)                                          \
	godot::ClassDB::bind_method(godot::D_METHOD(#NAME, __VA_ARGS__),           \
								&CLASS::NAME);

class TerrainMap;

namespace ServerRemoteFunctions
{
// void SetUsername(std::string)
inline const std::string SetUsername = "SetUsername";

// void UpdatePlayer(tick, pos, vel, forward)
inline const std::string UpdatePlayer = "UpdatePlayer";

// void GetRealms(void)
inline const std::string GetRealms = "GetRealms";

// void JoinRealm(std::string)
inline const std::string JoinRealm = "JoinRealm";

// void GetEntitiesData({uint64_t entityId}, ...)
inline const std::string GetEntitiesData = "GetEntitiesData";

// void GetTerrain(void)
inline const std::string GetTerrain = "GetTerrain";
} // namespace ServerRemoteFunctions

class ClientConnection : public godot::Node
{
	GDCLASS(ClientConnection, Node)
public:
	static void _bind_methods();

	void _enter_tree() override;
	
	godot::Array GetRealms();
	void RefreshRealms();
	void SetUsername(const godot::String &userName);
	void EnterRealm(const godot::String &realmName);
	
	void Connect(const godot::String &realmName, int64_t port);
	
	void _OnConnected(RpcClient *client);

public:
	std::vector<std::string> realms;
	
public:
	RpcHost *rpcHost = nullptr;
	icon7::RPCEnvironment *rpc = nullptr;
	RpcClient *peer = nullptr;
	std::string username;
	uint64_t playerEntityId = 0;
	
	RpcClient *GetClient();
	void SetClient(RpcClient *peer);
	RpcHost *GetHost();
	void SetHost(RpcHost *peer);

private:
	void RegisterMessages();

	void UpdateTerrain(TerrainMap &map);
	void SetRealms(std::vector<std::string> &realms);

	// 	{entityId, lastUpdateTick, vel, pos, forward, height,
	// 	width, maxMovementHorizontalSpeer, movable, modelName, userName}, ...)
	void SpawnEntities(icon7::ByteReader *reader);

	// 	{entityId, lastUpdateTick, vel, pos, forward}, ...)
	void UpdateEntities(icon7::ByteReader *reader);

	void SetModel(uint64_t entityId, std::string_view modelName, float height,
				  float width);

	void DeleteEntities(icon7::ByteReader *reader);
	void SetPlayerEntityId(uint64_t playerEntityId);
};
