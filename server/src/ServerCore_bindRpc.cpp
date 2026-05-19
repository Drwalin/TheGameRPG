#include <functional>

#include "../../ICon7/include/icon7/RPCEnvironment.hpp"
#include "../../ICon7/include/icon7/Flags.hpp"

#include "../../common/include/ServerRpcFunctionNames.hpp"

#include "../include/ClientRpcProxy.hpp"

#include "../include/ServerCore.hpp"

void ServerCore::BindRpc()
{
	std::function<icon7::CommandExecutionQueue *(
			icon7::MessageConverter *messageConverter, icon7::PeerHandle peer,
			icon7::ByteReader &reader, icon7::Flags flags)> executionQueueSelector;
	
	executionQueueSelector = std::bind(std::mem_fn(&ServerCore::SelectExecutionQueueByRealm), this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4);
	
	rpc->RegisterObjectMessage(ServerRpcFunctionNames::Login, this,
							   &ServerCore::Login);

	rpc->RegisterMessage(ServerRpcFunctionNames::UpdatePlayer,
						 &ServerCore::UpdatePlayer, nullptr,
						 executionQueueSelector);

	rpc->RegisterMessage(ServerRpcFunctionNames::GetEntitiesData,
						 &ServerCore::RequestSpawnEntities, nullptr,
						 executionQueueSelector);

	rpc->RegisterMessage(
		ServerRpcFunctionNames::Ping,
		[](icon7::PeerHandle peer, icon7::Flags flags, int64_t payload, int64_t payload2, int64_t payload3) {
			ClientRpcProxy::Pong(peer, flags, payload, payload2, payload3);
		},
		nullptr, nullptr);

	rpc->RegisterObjectMessage(ServerRpcFunctionNames::InteractInLineOfSight,
							   this, &ServerCore::InteractInLineOfSight,
							   nullptr, executionQueueSelector);
	rpc->RegisterObjectMessage(ServerRpcFunctionNames::Attack, this,
							   &ServerCore::Attack, nullptr,
							   executionQueueSelector);
}
