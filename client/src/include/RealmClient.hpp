#pragma once

#include <godot_cpp/godot.hpp>
#include <godot_cpp/core/defs.hpp>
#include <godot_cpp/classes/packed_scene.hpp>

#include <icon7/RPCEnvironment.hpp>
#include <icon7/Peer.hpp>

#include "../../../common/include/Realm.hpp"

class ClientConnection;

class RealmClient : public Realm
{
public:
	RealmClient();
	virtual ~RealmClient() override;

	virtual void Init(const std::string &realmName) override;
	void Reinit(const std::string &realmName);

	// returns false if was not busy
	virtual bool OneEpoch() override;

	void ConnectPeer(icon7::Peer *peer);
	void DisconnectPeer(icon7::Peer *peer);

	void ExecuteOnRealmThread(icon7::Peer *peer, icon7::ByteReader *reader,
							  void (*function)(icon7::Peer *,
											   std::vector<uint8_t> &, void *));

	virtual void RegisterObservers() override;
	virtual void RegisterSystems() override;
	
public:
	ClientConnection *clientConnection;
	icon7::RPCEnvironment *rpc;
	
	godot::Ref<godot::PackedScene> entityPrefab;
};
