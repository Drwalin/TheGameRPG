#pragma once

#include "../../client/include/GameClient.hpp"

class GameClientFrontend : public GameClient
{
public:
	GameClientFrontend(class GameFrontend *frontend);
	virtual ~GameClientFrontend() override;

	void Init();
	virtual void RegisterObservers() override;

public: // callbacks
	virtual void OnEnterRealm(const std::string &realmName) override;

	virtual void OnSetPlayerId(uint64_t localId) override;
	virtual void OnPlayerIdUnset() override;

	virtual void LoginFailed(std::string reason) override;
	virtual void LoginSuccessfull() override;

	virtual void RunOneEpoch() override;

	virtual bool GetCollisionShape(std::string collisionShapeName,
								   TerrainCollisionData *data) override;

public:
	class GameFrontend *frontend;

public:
	static GameClientFrontend *singleton;
};
