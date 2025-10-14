#pragma once

#include "../../ICon7/include/icon7/Forward.hpp"

#include "../../common/include/Realm.hpp"

class GameClient;

class RealmClient : public Realm
{
public:
	RealmClient(GameClient *gameClient);
	virtual ~RealmClient() override;

	inline constexpr static int64_t TICKS_UPDATE_DELAY = 3;
	inline constexpr static icon7::time::Diff STATE_UPDATE_DELAY = icon7::time::milliseconds(TICK_DURATION_MILLISECONDS*TICKS_UPDATE_DELAY);
	inline constexpr static icon7::time::Diff BASE_MAX_CORRECTION_OF_NEXT_TICK = -icon7::time::milliseconds(TICK_DURATION_MILLISECONDS/3);

	virtual void Init(const std::string &realmName) override;
	virtual void Clear() override;
	void Reinit(const std::string &realmName);

	void AddNewAuthoritativeMovementState(
		uint64_t localId, uint64_t serverId,
		ComponentMovementState state, Tick tick);
	void UpdateEntityCurrentState(uint64_t localId, uint64_t serverId);

	virtual ComponentMovementState ExecuteMovementUpdate(uint64_t entityId) override;

	void RegisterObservers();

protected:
	// returns false if was not busy
	virtual void OneEpoch() override;

public:
	GameClient *gameClient;
	icon7::RPCEnvironment *rpc;
};
