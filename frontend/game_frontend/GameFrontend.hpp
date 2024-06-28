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
#include <godot_cpp/classes/camera3d.hpp>

#include <glm/glm.hpp>

#include "GameClientFrontend.hpp"

using namespace godot;

class GameFrontend : public Node
{
	GDCLASS(GameFrontend, Node)
public: // Godot bound functions
	GameFrontend();
	GameFrontend(GameFrontend &) = delete;
	GameFrontend(GameFrontend &&) = delete;
	GameFrontend(const GameFrontend &) = delete;
	virtual ~GameFrontend() override;

	static void _bind_methods();

	void _ready() override;
	void _process(double dt) override;

	// call it from within _ready script-overriden method
	void InternalReady();
	// call it from within _process script-overriden method
	void InternalProcess();

	void Connect(const String &ip, int64_t port);
	void Login(const String &username, const String &password);
	void Disconnect();

	Node *GetNodeToAddEntities();
	Node *GetNodeToAddStaticMap();

public:
	void OnLoginFailed(std::string reason);
	void OnLoginSuccessfull();

public: // Call functions from godot to game
	void SetPlayerDirectionMovement(const Vector2 &dir);
	void PlayerTryJump();
	void SetPlayerRotation(const Vector3 &rot);
	Vector3 GetPlayerRotation();
	Vector3 GetPlayerPosition();
	Vector3 GetPlayerVelocity();
	float GetPlayerHeight();
	float GetPlayerWidth();
	Camera3D *GetPlayerCamera();
	bool GetIsPlayerOnGround();
	bool IsInPlayerControl();
	void PerformInteractionUse();

	bool IsConnected();
	bool IsDisconnected();

public: // variables
	GameClientFrontend *client = nullptr;
	Camera3D *playerCamera = nullptr;

	Node *entitiesContainer = nullptr;
	Node *staticMapContainer = nullptr;
	Node *nodeUI = nullptr;
};
