#include <icon7/Flags.hpp>
#include <icon7/RPCEnvironment.hpp>

#include "../../common/include/ClientRpcFunctionNames.hpp"

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

void Pong(icon7::Peer *peer, icon7::Flags flags, int64_t data)
{
	// TODO: correct
	PeerData *peerData = ((PeerData *)(peer->userPointer));
	int64_t currentTick = 0;
	if (peerData) {
		auto realm = peerData->realm.lock();
		if (realm.get() != nullptr) {
			currentTick = realm->timer.CalcCurrentTick();
		} else {
			LOG_DEBUG("Peer has no realm");
		}
	} else {
		LOG_DEBUG("No peer data");
	}

	LOG_DEBUG("Pong %lu %lu", data, currentTick);
	peer->host->GetRpcEnvironment()->Send(
		peer, flags, ClientRpcFunctionNames::Pong, data, currentTick);
}

void SetGravity(RealmServer *realm, icon7::Peer *peer, float gravity)
{
	realm->rpc->Send(peer, icon7::FLAG_RELIABLE,
					 ClientRpcFunctionNames::SetGravity, gravity);
}

void DeleteEntity_ForPeer(std::shared_ptr<RealmServer> realm, icon7::Peer *peer,
						  uint64_t entityId)
{
	realm->rpc->Send(peer, icon7::FLAG_RELIABLE,
					 ClientRpcFunctionNames::DeleteEntities, entityId);
}

void SpawnEntities_ForPeer(std::shared_ptr<RealmServer> realm,
						   icon7::Peer *peer)
{
	icon7::ByteWriter writer(1000);
	realm->rpc->InitializeSerializeSend(writer,
										ClientRpcFunctionNames::SpawnEntities);
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
	icon7::Flags flags = icon7::FLAG_RELIABLE | icon7::FLAGS_CALL_NO_FEEDBACK;
	realm->rpc->FinalizeSerializeSend(writer, flags);
	peer->Send(std::move(writer.Buffer()));
}

void SpawnEntities_ForPeerByIds(std::shared_ptr<RealmServer> realm,
								icon7::Peer *peer, icon7::ByteReader &reader)
{
	icon7::ByteWriter writer(1000);
	realm->rpc->InitializeSerializeSend(writer,
										ClientRpcFunctionNames::SpawnEntities);
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

				writer.op(*entity.get<EntityLastAuthoritativeMovementState>());
				writer.op(*entity.get<EntityName>());
				writer.op(*entity.get<EntityModelName>());
				writer.op(*entity.get<EntityShape>());
				writer.op(*entity.get<EntityMovementParameters>());
			}
		}
	}
	icon7::Flags flags = icon7::FLAG_RELIABLE | icon7::FLAGS_CALL_NO_FEEDBACK;
	realm->rpc->FinalizeSerializeSend(writer, flags);
	peer->Send(std::move(writer.Buffer()));
}

void Broadcast_SetModel(std::shared_ptr<RealmServer> realm, uint64_t entityId,
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

void Broadcast_UpdateEntities(std::shared_ptr<RealmServer> realm)
{
	const static uint32_t singleEntitySize = 8 + 8 + 12 + 12 + 12 + 1;

	int written = 0;

	icon7::ByteWriter writer(1500);

	realm->queryLastAuthoritativeState.each(
		[&](flecs::entity entity,
			const EntityLastAuthoritativeMovementState &state) {
			if (written == 0) {
				writer.Reinit(1500);
				realm->rpc->InitializeSerializeSend(
					writer, ClientRpcFunctionNames::UpdateEntities);
			}

			writer.op((uint64_t)entity.id());
			writer.op(state);
			written++;

			if (writer.GetSize() + singleEntitySize >= 1100) {
				icon7::Flags flags =
					icon7::FLAG_UNRELIABLE | icon7::FLAGS_CALL_NO_FEEDBACK;
				realm->rpc->FinalizeSerializeSend(writer, flags);
				icon7::ByteBuffer buffer = std::move(writer.Buffer());
				realm->Broadcast(buffer, 0);
				written = 0;
			}
		});
	if (written > 0) {
		icon7::Flags flags =
			icon7::FLAG_UNRELIABLE | icon7::FLAGS_CALL_NO_FEEDBACK;
		realm->rpc->FinalizeSerializeSend(writer, flags);
		realm->Broadcast(writer.Buffer(), 0);
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
