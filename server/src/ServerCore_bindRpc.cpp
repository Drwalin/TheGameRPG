#include "../../thirdparty/flecs/distr/flecs.h"

#include "../../ICon7/include/icon7/RPCEnvironment.hpp"
#include "../../ICon7/include/icon7/Flags.hpp"

#include "../../common/include/ServerRpcFunctionNames.hpp"

#include "../include/ClientRpcProxy.hpp"

#include "../include/ServerCore.hpp"

void ServerCore::BindRpc()
{
	rpc->RegisterObjectMessage(ServerRpcFunctionNames::Login, this,
							   &ServerCore::Login);

	rpc->RegisterMessage(ServerRpcFunctionNames::UpdatePlayer,
						 &ServerCore::UpdatePlayer, nullptr,
						 SelectExecutionQueueByRealm);

	rpc->RegisterMessage(ServerRpcFunctionNames::GetEntitiesData,
						 &ServerCore::RequestSpawnEntities, nullptr,
						 SelectExecutionQueueByRealm);

	rpc->RegisterMessage(
		ServerRpcFunctionNames::Ping,
		[](icon7::Peer *peer, icon7::Flags flags, int64_t payload) {
			ClientRpcProxy::Pong(peer, flags, payload);
		},
		nullptr, nullptr);

	rpc->RegisterObjectMessage(ServerRpcFunctionNames::InteractInLineOfSight,
							   this, &ServerCore::InteractInLineOfSight,
							   nullptr, SelectExecutionQueueByRealm);
	rpc->RegisterObjectMessage(ServerRpcFunctionNames::Attack, this,
							   &ServerCore::Attack, nullptr,
							   SelectExecutionQueueByRealm);
}
