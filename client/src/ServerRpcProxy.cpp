#include <icon7/Flags.hpp>
#include <icon7/RPCEnvironment.hpp>

#include <ClientRpcFunctionNames.hpp>

#include "../include/RealmClient.hpp"
#include "../include/GameClient.hpp"
#include "ServerRpcFunctionNames.hpp"

#include "../include/ServerRpcProxy.hpp"

namespace ServerRpcProxy
{
void Login(GameClient *gameClient, std::string username, std::string password)
{
	gameClient->rpc->Send(gameClient->peer.get(),
						  icon7::FLAG_RELIABLE | icon7::FLAGS_CALL_NO_FEEDBACK,
						  ServerRpcFunctionNames::Login, username, password);
}

void UpdatePlayer(GameClient *gameClient, const ComponentMovementState &state)
{
	gameClient->rpc->Send(gameClient->peer.get(),
						  icon7::FLAG_RELIABLE | icon7::FLAGS_CALL_NO_FEEDBACK,
						  ServerRpcFunctionNames::UpdatePlayer, state);
}

void GetEntitiesData(GameClient *gameClient,
					 const std::vector<uint64_t> &entities)
{
	icon7::ByteWriter writer(1000);
	gameClient->rpc->InitializeSerializeSend(
		writer, ServerRpcFunctionNames::GetEntitiesData);
	writer.op(entities.data(), entities.size());
	icon7::Flags flags = icon7::FLAG_RELIABLE | icon7::FLAGS_CALL_NO_FEEDBACK;
	gameClient->rpc->FinalizeSerializeSend(writer, flags);
	gameClient->peer->Send(std::move(writer.Buffer()));
}

void Ping(GameClient *gameClient, bool reliable)
{
	int64_t currentTick = gameClient->pingTimer.CalcCurrentTick();
	gameClient->rpc->Send(
		gameClient->peer.get(),
		(reliable ? icon7::FLAG_RELIABLE : icon7::FLAG_UNRELIABLE) |
			icon7::FLAGS_CALL_NO_FEEDBACK,
		ServerRpcFunctionNames::Ping, currentTick);
}

void InteractInLineOfSight(GameClient *gameClient, ComponentMovementState state,
						   uint64_t targetId, glm::vec3 dstPos,
						   glm::vec3 normal)
{
	gameClient->rpc->Send(gameClient->peer.get(),
						  icon7::FLAG_RELIABLE | icon7::FLAGS_CALL_NO_FEEDBACK,
						  ServerRpcFunctionNames::InteractInLineOfSight, state,
						  targetId, dstPos, normal);
}

void Attack(GameClient *gameClient, ComponentMovementState state,
			uint64_t targetId, glm::vec3 targetPos, int64_t attackType,
			int64_t attackId, int64_t argInt)
{
	gameClient->rpc->Send(gameClient->peer.get(),
						  icon7::FLAG_RELIABLE | icon7::FLAGS_CALL_NO_FEEDBACK,
						  ServerRpcFunctionNames::Attack, state, targetId,
						  targetPos, attackType, attackId, argInt);
}
} // namespace ServerRpcProxy
