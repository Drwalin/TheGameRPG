#include <godot_cpp/classes/window.hpp>
#include "godot_cpp/classes/engine.hpp"

#include "GodotGlm.hpp"

#include "GameFrontend.hpp"

#define METHOD_NO_ARGS(CLASS, NAME)                                            \
	ClassDB::bind_method(D_METHOD(#NAME), &CLASS::NAME);

#define METHOD_ARGS(CLASS, NAME, ...)                                          \
	ClassDB::bind_method(D_METHOD(#NAME, __VA_ARGS__), &CLASS::NAME);

GameFrontend::GameFrontend()
{
	client = nullptr;
	if (Engine::get_singleton()->is_editor_hint()) {
		return;
	}
}

GameFrontend::~GameFrontend()
{
	if (client) {
		client->Destroy();
		delete client;
		client = nullptr;
	}
}

void GameFrontend::_bind_methods()
{
	METHOD_ARGS(GameFrontend, Connect, "ip", "port");
	METHOD_ARGS(GameFrontend, Login, "username", "password");
	METHOD_NO_ARGS(GameFrontend, Disconnect);

	METHOD_NO_ARGS(GameFrontend, GetNodeToAddEntities);
	METHOD_NO_ARGS(GameFrontend, GetNodeToAddStaticMap);

	METHOD_ARGS(GameFrontend, SetPlayerDirectionMovement, "movementDir");
	METHOD_NO_ARGS(GameFrontend, PlayerTryJump);
	METHOD_ARGS(GameFrontend, SetPlayerRotation, "rotation");
	METHOD_NO_ARGS(GameFrontend, GetPlayerRotation);
	METHOD_NO_ARGS(GameFrontend, GetPlayerPosition);
	METHOD_NO_ARGS(GameFrontend, GetPlayerVelocity);
	
	METHOD_NO_ARGS(GameFrontend, GetIsPlayerOnGround);
	
	METHOD_NO_ARGS(GameFrontend, GetPlayerHeight);
	METHOD_NO_ARGS(GameFrontend, GetPlayerWidth);
	METHOD_NO_ARGS(GameFrontend, GetPlayerCamera);

	METHOD_NO_ARGS(GameFrontend, IsConnected);
	METHOD_NO_ARGS(GameFrontend, IsDisconnected);

	METHOD_NO_ARGS(GameFrontend, InternalReady);
	METHOD_NO_ARGS(GameFrontend, InternalProcess);
}

void GameFrontend::_ready() { InternalReady(); }
void GameFrontend::_process(double dt) { InternalProcess(); }
void GameFrontend::InternalReady()
{
	if (Engine::get_singleton()->is_editor_hint()) {
		return;
	}

	client = new GameClientFrontend(this);
	client->Init();
	playerCamera =
		(Camera3D *)(this->get_node_or_null("/root/SceneRoot/PlayerCamera3D"));
	entitiesContainer =
		(Node *)(this->get_node_or_null("/root/SceneRoot/EntitiesContainer"));
	staticMapContainer =
		(Node *)(this->get_node_or_null("/root/SceneRoot/Terrain"));
}

void GameFrontend::InternalProcess()
{
	if (Engine::get_singleton()->is_editor_hint()) {
		return;
	}

	client->RunOneEpoch();
}

void GameFrontend::Connect(const String &ip, int64_t port)
{
	client->ConnectToServer(ip.utf8().ptr(), port);
}

void GameFrontend::Login(const String &username, const String &password)
{
	client->Login(username.utf8().ptr(), password.utf8().ptr());
}

void GameFrontend::Disconnect() { client->DisconnectRealmPeer(); }

Node *GameFrontend::GetNodeToAddEntities() { return entitiesContainer; }

Node *GameFrontend::GetNodeToAddStaticMap() { return staticMapContainer; }

void GameFrontend::SetPlayerDirectionMovement(const Vector2 &dir)
{
	client->ProvideMovementInputDirection(ToGlm(dir));
}

void GameFrontend::PlayerTryJump() { client->TryPerformJump(); }

void GameFrontend::SetPlayerRotation(const Vector3 &rot)
{
	client->SetRotation(ToGlm(rot));
}

Vector3 GameFrontend::GetPlayerRotation()
{
	return ToGodot(client->GetRotation());
}

Vector3 GameFrontend::GetPlayerPosition()
{
	return ToGodot(client->GetPosition());
}

Vector3 GameFrontend::GetPlayerVelocity()
{
	return ToGodot(client->GetVelocity());
}

bool GameFrontend::GetIsPlayerOnGround()
{
	return client->GetOnGround();
}

float GameFrontend::GetPlayerHeight() { return client->GetShape().height; }
float GameFrontend::GetPlayerWidth() { return client->GetShape().width; }

Camera3D *GameFrontend::GetPlayerCamera() { return playerCamera; }

bool GameFrontend::IsConnected() { return client->IsConnected(); }

bool GameFrontend::IsDisconnected() { return client->IsDisconnected(); }
