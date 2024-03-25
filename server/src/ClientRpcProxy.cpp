#include "../include/RealmServer.hpp"
#include "../include/ClientRpcFunctionNames.hpp"

#include "../include/ClientRpcProxy.hpp"
#include "icon7/Flags.hpp"

void SetRealms(RealmServer *realm, icon7::Peer *peer,
			   const std::vector<std::string> &realmNames)
{
	realm->rpc->Send(peer, icon7::FLAG_RELIABLE,
					 ClientRpcFunctionNames::SetRealms, realmNames);
}

void SetPlayerEntityId(RealmServer *realm, icon7::Peer *peer,
					   uint64_t playerEntityId)
{
	realm->rpc->Send(peer, icon7::FLAG_RELIABLE,
					 ClientRpcFunctionNames::SetPlayerEntityId, playerEntityId);
}

void SetCurrentTick(RealmServer *realm, icon7::Peer *peer)
{
	realm->rpc->Send(peer, icon7::FLAG_RELIABLE,
					 ClientRpcFunctionNames::SetCurrentTick,
					 realm->timer.currentTick);
}

void Ping(RealmServer *realm, icon7::Peer *peer)
{
	realm->rpc->Send(peer, icon7::FLAG_RELIABLE, ClientRpcFunctionNames::Ping,
					 realm->timer.currentTick);
}

void SetGravity(RealmServer *realm, icon7::Peer *peer, float gravity)
{
	realm->rpc->Send(peer, icon7::FLAG_RELIABLE,
					 ClientRpcFunctionNames::SetGravity, gravity);
}

void DeleteEntity_ForPeer(RealmServer *realm, icon7::Peer *peer,
						  uint64_t entityId)
{
	realm->rpc->Send(peer, icon7::FLAG_RELIABLE,
					 ClientRpcFunctionNames::DeleteEntities, entityId);
}

void SpawnEntities_ForPeer(RealmServer *realm, icon7::Peer *peer)
{
	std::vector<uint8_t> buffer;
	{
		bitscpp::ByteWriter writer(buffer);
		writer.op(ClientRpcFunctionNames::SpawnEntities);
		realm->queryEntityLongState.each(
			[&](flecs::entity entity,
				const EntityLastAuthoritativeMovementState state,
				const EntityName name, const EntityModelName model,
				const EntityShape shape,
				const EntityMovementParameters movementParams)
		{
			writer.op((uint64_t)entity.id());
			writer.op(state);
			writer.op(name);
			writer.op(model);
			writer.op(shape);
			writer.op(movementParams);
		});
	}
	peer->Send(std::move(buffer), icon7::FLAG_RELIABLE | icon7::FLAGS_CALL_NO_FEEDBACK);
}

void Broadcast_SetModel(RealmServer *realm, uint64_t entityId,
						const std::string &modelName, EntityShape shape);
void Broadcast_SpawnEntity(RealmServer *realm, flecs::entity entity,
						   const EntityMovementState &state,
						   const EntityShape &shape,
						   const EntityModelName &entityModelName,
						   const EntityName &entityName);
void Broadcast_UpdateEntities(RealmServer *realm);
void Broadcast_DeleteEntity(RealmServer *realm, uint64_t entityId);
