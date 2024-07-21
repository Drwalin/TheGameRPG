#pragma once

#include "../../ICon7/include/icon7/Forward.hpp"

#include "../../common/include/Realm.hpp"

class GameClient;

class RealmClient : public Realm
{
public:
	RealmClient(GameClient *gameClient);
	virtual ~RealmClient() override;

	inline const static int64_t STATE_UPDATE_DELAY = 100;

	virtual void Init(const std::string &realmName) override;
	virtual void Clear() override;
	void Reinit(const std::string &realmName);

	// returns false if was not busy
	virtual bool OneEpoch() override;

	void AddNewAuthoritativeMovementState(
		uint64_t localId, uint64_t serverId,
		ComponentLastAuthoritativeMovementState state);
	void UpdateEntityCurrentState(uint64_t localId, uint64_t serverId);

	virtual void ExecuteMovementUpdate(uint64_t entityId,
									   ComponentMovementState *state) override;

	void RegisterObservers();

	virtual bool GetCollisionShape(std::string collisionShapeName,
								   TerrainCollisionData *data) override;

public:
	GameClient *gameClient;
	icon7::RPCEnvironment *rpc;
};
