#pragma once

#include <string>

#include "../../ICon7/include/icon7/Peer.hpp"

class ServerCore;

namespace peer_transitions
{
// TODO: Rename to something better, i.e.: InitiateConnectingToRealm
void OnReceivedLogin(ServerCore *serverCore, icon7::Peer *peer, const std::string &username);
} // namespace peer_transitions
