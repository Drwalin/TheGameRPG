#pragma once

#include <string>

#include "../../ICon7/include/icon7/Peer.hpp"

namespace peer_transitions
{
// TODO: Rename to something better, i.e.: InitiateConnectingToRealm
void OnReceivedLogin(icon7::Peer *peer, const std::string &username);
} // namespace peer_transitions
