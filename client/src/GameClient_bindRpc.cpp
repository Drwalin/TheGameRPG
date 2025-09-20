#include "../../ICon7/include/icon7/RPCEnvironment.hpp"

#include "../../common/include/ClientRpcFunctionNames.hpp"

#include "../include/ServerRpcProxy.hpp"

#include "../include/GameClient.hpp"

void GameClient::BindRpc()
{
	rpc->RegisterObjectMessage(ClientRpcFunctionNames::JoinRealm, this,
							   &GameClient::JoinRealm, &executionQueue);
	rpc->RegisterObjectMessage(ClientRpcFunctionNames::SpawnEntities, this,
							   &GameClient::SpawnEntities, &executionQueue);
	rpc->RegisterObjectMessage(ClientRpcFunctionNames::SpawnStaticEntities,
							   this, &GameClient::SpawnStaticEntities,
							   &executionQueue);
	rpc->RegisterObjectMessage(ClientRpcFunctionNames::UpdateEntities, this,
							   &GameClient::UpdateEntities, &executionQueue);
	rpc->RegisterObjectMessage(ClientRpcFunctionNames::SetModel, this,
							   &GameClient::SetModel, &executionQueue);
	rpc->RegisterObjectMessage(ClientRpcFunctionNames::DeleteEntities, this,
							   &GameClient::DeleteEntities, &executionQueue);
	rpc->RegisterObjectMessage(ClientRpcFunctionNames::SetPlayerEntityId, this,
							   &GameClient::SetPlayerEntityId, &executionQueue);
	rpc->RegisterObjectMessage(ClientRpcFunctionNames::SetGravity, this,
							   &GameClient::SetGravity, &executionQueue);
	rpc->RegisterObjectMessage(ClientRpcFunctionNames::LoginFailed, this,
							   &GameClient::LoginFailed, &executionQueue);
	rpc->RegisterObjectMessage(ClientRpcFunctionNames::LoginSuccessfull, this,
							   &GameClient::LoginSuccessfull, &executionQueue);
	rpc->RegisterObjectMessage(ClientRpcFunctionNames::Pong, this,
							   &GameClient::Pong, &executionQueue);
	rpc->RegisterObjectMessage(ClientRpcFunctionNames::GenericComponentUpdate,
							   this, &GameClient::GenericComponentUpdate,
							   &executionQueue);
	rpc->RegisterObjectMessage(
		ClientRpcFunctionNames::PlayDeathAndDestroyEntity, this,
		&GameClient::PlayDeathAndDestroyEntity, &executionQueue);
	rpc->RegisterObjectMessage(ClientRpcFunctionNames::PlayAnimation, this,
							   &GameClient::PlayAnimation, &executionQueue);
	rpc->RegisterObjectMessage(ClientRpcFunctionNames::PlayFX, this,
							   &GameClient::PlayFX, &executionQueue);
}
