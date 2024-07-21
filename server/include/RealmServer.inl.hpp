#pragma once

#include <icon7/RPCEnvironment.hpp>

#include "RealmServer.hpp"

template <typename... Args>
inline void RealmServer::BroadcastReliable(const std::string &functionName,
										   Args &&...args)
{
	Broadcast(icon7::FLAG_RELIABLE, functionName, std::forward<Args>(args)...);
}

template <typename... Args>
inline void RealmServer::BroadcastUnreliable(const std::string &functionName,
											 Args &&...args)
{
	Broadcast(icon7::FLAG_UNRELIABLE, functionName,
			  std::forward<Args>(args)...);
}

template <typename... Args>
inline void RealmServer::Broadcast(icon7::Flags flags,
								   const std::string &functionName,
								   Args &&...args)
{
	icon7::ByteBuffer buffer(1500);
	flags |= icon7::FLAGS_CALL_NO_FEEDBACK;
	rpc->SerializeSend(buffer, flags, functionName,
					   std::forward<Args>(args)...);
	Broadcast(buffer, 0);
}

template <typename... Args>
inline void RealmServer::BroadcastReliableExcept(
	uint64_t exceptEntityId, const std::string &functionName, Args &&...args)
{
	BroadcastExcept(exceptEntityId, icon7::FLAG_RELIABLE, functionName,
					std::forward<Args>(args)...);
}

template <typename... Args>
inline void RealmServer::BroadcastUnreliableExcept(
	uint64_t exceptEntityId, const std::string &functionName, Args &&...args)
{
	BroadcastExcept(exceptEntityId, icon7::FLAG_UNRELIABLE, functionName,
					std::forward<Args>(args)...);
}

template <typename... Args>
inline void
RealmServer::BroadcastExcept(uint64_t exceptEntityId, icon7::Flags flags,
							 const std::string &functionName, Args &&...args)
{
	icon7::ByteBuffer buffer(1500);
	flags |= icon7::FLAGS_CALL_NO_FEEDBACK;
	rpc->SerializeSend(buffer, flags, functionName,
					   std::forward<Args>(args)...);
	Broadcast(buffer, exceptEntityId);
}
