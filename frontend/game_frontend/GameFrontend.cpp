#include <godot_cpp/classes/window.hpp>
#include "godot_cpp/classes/engine.hpp"

#include "GodotGlm.hpp"

#include "GameFrontend.hpp"

#define METHOD_NO_ARGS(CLASS, NAME)                                            \
	ClassDB::bind_method(D_METHOD(#NAME), &CLASS::NAME);

#define METHOD_ARGS(CLASS, NAME, ...)                                          \
	ClassDB::bind_method(D_METHOD(#NAME, __VA_ARGS__), &CLASS::NAME);

GameFrontend::GameFrontend() {
	gameClient = nullptr;
	if (Engine::get_singleton()->is_editor_hint()) {
		return;
	}
}

GameFrontend::~GameFrontend() {
	if (gameClient) {
		gameClient->Destroy();
		delete gameClient;
		gameClient = nullptr;
	}
}

void GameFrontend::_bind_methods()
{
	METHOD_ARGS(GameFrontend, Connect, "ip", "port");
	METHOD_ARGS(GameFrontend, Login, "username", "password");
	METHOD_NO_ARGS(GameFrontend, Disconnect);

	METHOD_NO_ARGS(GameFrontend, GetNodeToAddEntities);

	METHOD_ARGS(GameFrontend, SetPlayerDirectionMovement, "movementDir");
	METHOD_NO_ARGS(GameFrontend, PlayerTryJump);
	METHOD_ARGS(GameFrontend, SetPlayerRotation, "rotation");
	METHOD_NO_ARGS(GameFrontend, GetPlayerRotation);
	METHOD_NO_ARGS(GameFrontend, GetPlayerPosition);
	METHOD_NO_ARGS(GameFrontend, GetPlayerVelocity);
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

	gameClient = new GameClientFrontend(this);
	gameClient->Init();
	playerCamera =
		(Camera3D *)(this->get_node_or_null("/root/SceneRoot/PlayerCamera3D"));
	entitiesContainer =
		(Node *)(this->get_node_or_null("/root/SceneRoot/EntitiesContainer"));
}

void GameFrontend::InternalProcess()
{
	if (Engine::get_singleton()->is_editor_hint()) {
		return;
	}

	gameClient->RunOneEpoch();
}

void GameFrontend::Connect(const String &ip, int64_t port)
{
	gameClient->ConnectToServer(ip.utf8().ptr(), port);
}

void GameFrontend::Login(const String &username, const String &password)
{
	gameClient->Login(username.utf8().ptr(), password.utf8().ptr());
}

void GameFrontend::Disconnect() { gameClient->DisconnectRealmPeer(); }

Node *GameFrontend::GetNodeToAddEntities() { return entitiesContainer; }

void GameFrontend::SetPlayerDirectionMovement(const Vector2 &dir)
{
	gameClient->ProvideMovementInputDirection(ToGlm(dir));
}

void GameFrontend::PlayerTryJump() { gameClient->TryPerformJump(); }

void GameFrontend::SetPlayerRotation(const Vector3 &rot)
{
	gameClient->SetRotation(ToGlm(rot));
}

Vector3 GameFrontend::GetPlayerRotation()
{
	return ToGodot(gameClient->GetRotation());
}

Vector3 GameFrontend::GetPlayerPosition()
{
	return ToGodot(gameClient->GetPosition());
}

Vector3 GameFrontend::GetPlayerVelocity()
{
	return ToGodot(gameClient->GetVelocity());
}

float GameFrontend::GetPlayerHeight()
{
	return gameClient->GetShape().height;
}
float GameFrontend::GetPlayerWidth()
{
	return gameClient->GetShape().width;
}

Camera3D *GameFrontend::GetPlayerCamera() { return playerCamera; }

bool GameFrontend::IsConnected()
{
	return  gameClient->IsConnected();
}

bool GameFrontend::IsDisconnected()
{
	return  gameClient->IsDisconnected();
}
