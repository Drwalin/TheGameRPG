#pragma once

#include <icon7/RPCEnvironment.hpp>
#include <icon7/Peer.hpp>

#include "../../common/include/EntityData.hpp"

class ClientCore;

namespace ServerRpcProxy
{
void SetUsername(ClientCore *clientCore, std::string username);

void UpdatePlayer(ClientCore *clientCore, EntityLastAuthoritativeMovementState &state);

void GetEntitiesData(ClientCore *clientCore, const std::vector<uint64_t> &entities);

void GetCurrentTick(ClientCore *clientCore);

void Ping(ClientCore *clientCore, uint64_t data);
}
