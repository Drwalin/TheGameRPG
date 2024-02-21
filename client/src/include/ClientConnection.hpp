#pragma once

#include <godot_cpp/variant/utility_functions.hpp>
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

#include "Rpc.hpp"
#include "Entities.hpp"

#define METHOD_NO_ARGS(CLASS, NAME)                                            \
	godot::ClassDB::bind_method(godot::D_METHOD(#NAME), &CLASS::NAME);

#define METHOD_ARGS(CLASS, NAME, ...)                                          \
	godot::ClassDB::bind_method(godot::D_METHOD(#NAME, __VA_ARGS__),           \
								&CLASS::NAME);

class TerrainMap;

class ClientConnection : public godot::Node
{
	GDCLASS(ClientConnection, Node)
public: // Godot bound functions
	ClientConnection();
	static void _bind_methods();

	void _enter_tree() override;
	
	RpcClient *GetClient();
	void SetClient(RpcClient *peer);
	RpcHost *GetHost();
	void SetHost(RpcHost *peer);
	
	godot::Array GetRealms();
	void RefreshRealms();
	void SetUsername(const godot::String &userName);
	void EnterRealm(const godot::String &realmName);
	
	void Connect(const godot::String &realmName, int64_t port);
	
private: // Godot callbacks
	void _OnConnected(RpcClient *client);

public: // variables
	RpcHost *rpcHost = nullptr;
	icon7::RPCEnvironment *rpc = nullptr;
	RpcClient *peer = nullptr;
	std::string username;
	uint64_t playerEntityId = 0;
	
	std::vector<std::string> realms;
	
	Entities entities;
	
private: //rpc code
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
