#include <godot_cpp/classes/window.hpp>

#include "GodotGlm.hpp"

#include "GameFrontend.hpp"

#define METHOD_NO_ARGS(CLASS, NAME)                                            \
	ClassDB::bind_method(D_METHOD(#NAME), &CLASS::NAME);

#define METHOD_ARGS(CLASS, NAME, ...)                                          \
	ClassDB::bind_method(D_METHOD(#NAME, __VA_ARGS__), &CLASS::NAME);

GameFrontend::GameFrontend() {}

GameFrontend::~GameFrontend() {}

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
	METHOD_NO_ARGS(GameFrontend, GetPlayerCamera);

	METHOD_NO_ARGS(GameFrontend, IsConnected);
	METHOD_NO_ARGS(GameFrontend, IsConnecting);

	METHOD_NO_ARGS(GameFrontend, InternalProcess);
}

void GameFrontend::_ready()
{
	gameClientFrontend = new GameClientFrontend(this);
	gameClientFrontend->Init();
	playerCamera =
		(Camera3D *)(this->get_node_or_null("/root/SceneRoot/PlayerCamera3D"));
	entitiesContainer =
		(Node *)(this->get_node_or_null("/root/SceneRoot/EntitiesContainer"));
	DEBUG("Entities container: %p", entitiesContainer);
	DEBUG("Camera: %p", playerCamera);
}

void GameFrontend::InternalProcess() { gameClientFrontend->RunOneEpoch(); }

void GameFrontend::Connect(const String &ip, int64_t port)
{
	gameClientFrontend->ConnectToServer(ip.utf8().ptr(), port);
}

void GameFrontend::Login(const String &username, const String &password)
{
	gameClientFrontend->Login(username.utf8().ptr(), password.utf8().ptr());
}

void GameFrontend::Disconnect() { gameClientFrontend->DisconnectRealmPeer(); }

Node *GameFrontend::GetNodeToAddEntities() { return entitiesContainer; }

void GameFrontend::SetPlayerDirectionMovement(const Vector2 &dir)
{
	gameClientFrontend->ProvideMovementInputDirection(ToGlm(dir));
}

void GameFrontend::PlayerTryJump() { gameClientFrontend->TryPerformJump(); }

void GameFrontend::SetPlayerRotation(const Vector3 &rot)
{
	gameClientFrontend->SetRotation(ToGlm(rot));
}

Vector3 GameFrontend::GetPlayerRotation()
{
	return ToGodot(gameClientFrontend->GetRotation());
}

Camera3D *GameFrontend::GetPlayerCamera() { return playerCamera; }

bool GameFrontend::IsConnected()
{
	if (gameClientFrontend->realmConnectionPeer != nullptr) {
		if (gameClientFrontend->realmConnectionPeer->IsReadyToUse()) {
			return true;
		}
	}
	return false;
}

bool GameFrontend::IsConnecting()
{
	if (gameClientFrontend->realmConnectionPeer != nullptr) {
		return true;
	}
	return false;
}
