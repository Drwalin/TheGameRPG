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
	GameClientFrontend(class GameFrontend *gameFrontend);
	virtual ~GameClientFrontend() override;

	void Init();

public: // callbacks
	virtual void OnEnterRealm(const std::string &realmName) override;

	virtual void OnEntityAdd(uint64_t localId) override;
	virtual void OnEntityRemove(uint64_t localId) override;
	virtual void OnEntityShape(uint64_t localId,
							   const EntityShape &shape) override;
	virtual void OnEntityModel(uint64_t localId,
							   const EntityModelName &model) override;

	virtual void OnSetPlayerId(uint64_t localId) override;
	virtual void OnPlayerIdUnset() override;

public:
	class GameFrontend *gameFrontend;
};
