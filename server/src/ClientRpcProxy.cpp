#include <icon7/Flags.hpp>
#include <icon7/RPCEnvironment.hpp>

#include <ClientRpcFunctionNames.hpp>

#include "../include/RealmServer.hpp"

#include "../include/ClientRpcProxy.hpp"

namespace ClientRpcProxy
{
void JoinRealm(RealmServer *realm, icon7::Peer *peer)
{
	realm->rpc->Send(peer, icon7::FLAG_RELIABLE,
					 ClientRpcFunctionNames::JoinRealm, realm->realmName);
}

void SetPlayerEntityId(RealmServer *realm, icon7::Peer *peer,
					   uint64_t playerEntityId)
{
	realm->rpc->Send(peer, icon7::FLAG_RELIABLE,
					 ClientRpcFunctionNames::SetPlayerEntityId, playerEntityId);
}

void Pong(icon7::Peer *peer, icon7::Flags flags, uint64_t data)
{
	// TODO: correct
	PeerData *peerData = ((PeerData *)(peer->userPointer));
	uint64_t currentTick = 0;
	if (peerData) {
		if (peerData->realm) {
			currentTick = peerData->realm->timer.CalcCurrentTick();
		} else {
			DEBUG("Peer has no realm");
		}
	} else {
		DEBUG("No peer data");
	}

	DEBUG("Pong %lu %lu", data, currentTick);
	peer->host->GetRpcEnvironment()->Send(
		peer, flags, ClientRpcFunctionNames::Pong, data, currentTick);
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

void SpawnEntities_ForPeerByIds(RealmServer *realm, icon7::Peer *peer,
								icon7::ByteReader &reader)
{
	std::vector<uint8_t> buffer;
	{
		bitscpp::ByteWriter writer(buffer);
		writer.op(ClientRpcFunctionNames::SpawnEntities);
		while (reader.get_remaining_bytes() >= 8) {
			uint64_t entityId = 0;
			reader.op(entityId);
			flecs::entity entity = realm->Entity(entityId);
			if (entity.is_alive()) {
				if (entity.has<EntityLastAuthoritativeMovementState>() &&
					entity.has<EntityName>() && entity.has<EntityModelName>() &&
					entity.has<EntityShape>() &&
					entity.has<EntityMovementParameters>()) {

					writer.op(entityId);

					writer.op(
						*entity.get<EntityLastAuthoritativeMovementState>());
					writer.op(*entity.get<EntityName>());
					writer.op(*entity.get<EntityModelName>());
					writer.op(*entity.get<EntityShape>());
					writer.op(*entity.get<EntityMovementParameters>());
				}
			}
		}
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

void Broadcast_SpawnEntity(RealmServer *realm, uint64_t entityId,
						   const EntityMovementState &state,
						   const EntityShape &shape,
						   const EntityModelName &entityModelName,
						   const EntityName &entityName)
{
	realm->BroadcastReliable(ClientRpcFunctionNames::SpawnEntities, entityId,
							 state, shape, entityModelName, entityName);
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

void LoginSuccessfull(icon7::Peer *peer)
{
	peer->host->GetRpcEnvironment()->Send(
		peer, icon7::FLAG_RELIABLE | icon7::FLAGS_CALL_NO_FEEDBACK,
		ClientRpcFunctionNames::LoginSuccessfull);
}

void LoginFailed(icon7::Peer *peer)
{
	peer->host->GetRpcEnvironment()->Send(
		peer, icon7::FLAG_RELIABLE | icon7::FLAGS_CALL_NO_FEEDBACK,
		ClientRpcFunctionNames::LoginFailed);
}
} // namespace ClientRpcProxy
