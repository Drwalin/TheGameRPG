#pragma once

#include <icon7/RPCEnvironment.hpp>
#include <icon7/Peer.hpp>

#include "../../common/include/Realm.hpp"

class ClientCore;

class RealmClient : public Realm
{
public:
	RealmClient();
	virtual ~RealmClient() override;

	virtual void Init(const std::string &realmName) override;
	virtual void Clear() override;
	void Reinit(const std::string &realmName);

	// returns false if was not busy
	virtual bool OneEpoch() override;

	virtual void RegisterObservers() override;
	virtual void RegisterSystems() override;

public:
	ClientCore *clientCore;
	icon7::RPCEnvironment *rpc;
	std::shared_ptr<icon7::Peer> peer;
};
