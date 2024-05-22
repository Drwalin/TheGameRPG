#pragma once

#include <string>

#include "../../ICon7/include/icon7/Peer.hpp"

namespace peer_transitions
{
void OnReceivedLogin(icon7::Peer *peer, const std::string &username);
}
