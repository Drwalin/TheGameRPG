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
	gameClient->rpc.Send(gameClient->realmConnectionPeer.get(),
						 icon7::FLAG_RELIABLE | icon7::FLAGS_CALL_NO_FEEDBACK,
						 ServerRpcFunctionNames::Login, username, password);
}

void UpdatePlayer(GameClient *gameClient, const EntityMovementState &state)
{
	gameClient->rpc.Send(gameClient->realmConnectionPeer.get(),
						 icon7::FLAG_RELIABLE | icon7::FLAGS_CALL_NO_FEEDBACK,
						 ServerRpcFunctionNames::UpdatePlayer, state);
}

void GetEntitiesData(GameClient *gameClient,
					 const std::vector<uint64_t> &entities)
{
	std::vector<uint8_t> buffer;
	{
		bitscpp::ByteWriter writer(buffer);
		writer.op(ServerRpcFunctionNames::GetEntitiesData);
		writer.op(entities.data(), entities.size());
	}

	gameClient->realmConnectionPeer->Send(std::move(buffer),
										  icon7::FLAG_RELIABLE |
											  icon7::FLAGS_CALL_NO_FEEDBACK);
}

void Ping(GameClient *gameClient, bool reliable)
{
	int64_t currentTick = gameClient->pingTimer.CalcCurrentTick();
	gameClient->rpc.Send(
		gameClient->realmConnectionPeer.get(),
		(reliable ? icon7::FLAG_RELIABLE : icon7::FLAG_UNRELIABLE) |
			icon7::FLAGS_CALL_NO_FEEDBACK,
		ServerRpcFunctionNames::Ping, currentTick);
}
} // namespace ServerRpcProxy
