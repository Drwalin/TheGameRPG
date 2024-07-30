#pragma once

#include <godot_cpp/variant/utility_functions.hpp>
#include <godot_cpp/godot.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <gdextension_interface.h>
#include <godot_cpp/core/defs.hpp>
#include <godot_cpp/core/engine_ptrcall.hpp>
#include <godot_cpp/classes/scene_tree.hpp>
#include <godot_cpp/classes/node2d.hpp>
#include <godot_cpp/variant/packed_byte_array.hpp>
#include <godot_cpp/classes/ref_counted.hpp>

#include "../../client/include/GameClient.hpp"

using namespace godot;

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
