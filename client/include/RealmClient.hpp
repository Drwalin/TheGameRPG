#pragma once

#include <icon7/RPCEnvironment.hpp>
#include <icon7/Peer.hpp>

#include "../../common/include/Realm.hpp"

class GameClient;

class RealmClient : public Realm
{
public:
	RealmClient(GameClient *gameClient);
	virtual ~RealmClient() override;

	virtual void Init(const std::string &realmName) override;
	virtual void Clear() override;
	void Reinit(const std::string &realmName);

	// returns false if was not busy
	virtual bool OneEpoch() override;

	void RegisterObservers();
	void RegisterSystems();

public:
	GameClient *gameClient;
	icon7::RPCEnvironment *rpc;
	std::shared_ptr<icon7::Peer> peer;
};
