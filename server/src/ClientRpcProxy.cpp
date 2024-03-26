#include "../include/RealmServer.hpp"
#include "../include/ClientRpcFunctionNames.hpp"

#include "../include/ClientRpcProxy.hpp"
#include "icon7/Flags.hpp"
#include "icon7/RPCEnvironment.hpp"

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
				const EntityMovementParameters movementParams) {
				writer.op((uint64_t)entity.id());
				writer.op(state);
				writer.op(name);
				writer.op(model);
				writer.op(shape);
				writer.op(movementParams);
			});
	}
	peer->Send(std::move(buffer),
			   icon7::FLAG_RELIABLE | icon7::FLAGS_CALL_NO_FEEDBACK);
}

void Broadcast_SetModel(RealmServer *realm, uint64_t entityId,
						const std::string &modelName, EntityShape shape)
{
	realm->BroadcastReliable(ClientRpcFunctionNames::SetModel, entityId,
							 modelName, shape);
}

void Broadcast_SpawnEntity(RealmServer *realm, flecs::entity entity,
						   const EntityMovementState &state,
						   const EntityShape &shape,
						   const EntityModelName &entityModelName,
						   const EntityName &entityName)
{
	realm->BroadcastReliable(ClientRpcFunctionNames::SpawnEntities,
							 (uint64_t)entity.id(), state, shape,
							 entityModelName, entityName);
}

void Broadcast_UpdateEntities(RealmServer *realm)
{
	const static uint32_t singleEntitySize = 8 + 8 + 12 + 12 + 12 + 1;

	int written = 0;

	std::vector<uint8_t> buffer;
	{
		bitscpp::ByteWriter writer(buffer);
		writer.op(ClientRpcFunctionNames::UpdateEntities);

		realm->queryLastAuthoritativeState.each(
			[&](flecs::entity entity,
				const EntityLastAuthoritativeMovementState &state) {
				if (writer.GetSize() + singleEntitySize >= 1100 &&
					written > 0) {
					writer.~ByteWriter();
					realm->Broadcast(buffer,
									 icon7::FLAG_UNRELIABLE |
										 icon7::FLAGS_CALL_NO_FEEDBACK,
									 0);
					buffer.clear();
					written = 0;
					new (&writer) bitscpp::ByteWriter(buffer);
					writer.op(ClientRpcFunctionNames::UpdateEntities);
				}

				writer.op((uint64_t)entity.id());
				writer.op(state);
			});
	}
	if (written > 0) {
		realm->Broadcast(
			buffer, icon7::FLAG_UNRELIABLE | icon7::FLAGS_CALL_NO_FEEDBACK, 0);
	}
}

void Broadcast_DeleteEntity(RealmServer *realm, uint64_t entityId)
{
	realm->BroadcastReliable(ClientRpcFunctionNames::DeleteEntities, entityId);
}
