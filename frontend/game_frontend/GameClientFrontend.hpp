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

public: // rpc callbacks
	virtual void PlayDeathAndDestroyEntity_virtual(ComponentModelName modelName,
												   ComponentMovementState state,
												   ComponentName name) override;
	virtual void PlayAnimation_virtual(uint64_t serverId,
									   ComponentModelName modelName,
									   ComponentMovementState state,
									   std::string currentAnimation,
									   int64_t animationStartTick) override;
	virtual void PlayFX(ComponentModelName modelName,
						ComponentStaticTransform transform,
						int64_t timeStartPlaying,
						uint64_t attachToEntityId) override;

public:
	class GameFrontend *frontend;

public:
	static GameClientFrontend *singleton;
};
