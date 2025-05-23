#include <icon7/Flags.hpp>
#include <icon7/RPCEnvironment.hpp>

#include "../../common/include/ClientRpcFunctionNames.hpp"
#include "../../common/include/EntityComponents.hpp"

#define ENABLE_REALM_SERVER_IMPLEMENTATION_TEMPLATE
#include "../include/RealmServer.hpp"

#include "../include/EntityGameComponents.hpp"
#include "../include/ClientRpcProxy.hpp"

namespace ClientRpcProxy
{
void JoinRealm(RealmServer *realm, icon7::Peer *peer, uint64_t playerEntityId)
{
	realm->rpc->Send(peer, icon7::FLAG_RELIABLE,
					 ClientRpcFunctionNames::JoinRealm, realm->realmName,
					 realm->timer.currentTick, playerEntityId);
}

void SetPlayerEntityId(RealmServer *realm, icon7::Peer *peer,
					   uint64_t playerEntityId)
{
	realm->rpc->Send(peer, icon7::FLAG_RELIABLE,
					 ClientRpcFunctionNames::SetPlayerEntityId, playerEntityId);
}

void Pong(icon7::Peer *peer, icon7::Flags flags, int64_t data)
{
	// TODO: verify and correct this behavior and implement both ways ping
	//       testing
	PeerData *peerData = ((PeerData *)(peer->userPointer));
	int64_t currentTick = 0;
	if (peerData) {
		auto realm = peerData->realm.lock();
		if (realm.get() != nullptr) {
			currentTick = realm->timer.CalcCurrentTick();
		}
	}

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
	icon7::ByteWriter writer(1000);
	realm->rpc->InitializeSerializeSend(writer,
										ClientRpcFunctionNames::SpawnEntities);
	realm->queryEntityLongState.each(
		[&](flecs::entity entity,
			const ComponentLastAuthoritativeMovementState state,
			const ComponentName name, const ComponentModelName model,
			const ComponentShape shape,
			const ComponentMovementParameters movementParams) {
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

void SpawnEntities_ForPeerByIds(RealmServer *realm, icon7::Peer *peer,
								icon7::ByteReader &reader)
{
	icon7::ByteWriter writer(1000);
	realm->rpc->InitializeSerializeSend(writer,
										ClientRpcFunctionNames::SpawnEntities);
	while (reader.get_remaining_bytes() >= 8) {
		uint64_t entityId = 0;
		reader.op(entityId);
		flecs::entity entity = realm->Entity(entityId);
		if (entity.is_alive()) {
			if (entity.has<ComponentLastAuthoritativeMovementState>() &&
				entity.has<ComponentName>() &&
				entity.has<ComponentModelName>() &&
				entity.has<ComponentShape>() &&
				entity.has<ComponentMovementParameters>()) {

				writer.op(entityId);

				writer.op(
					*entity.get<ComponentLastAuthoritativeMovementState>());
				writer.op(*entity.get<ComponentName>());
				writer.op(*entity.get<ComponentModelName>());
				writer.op(*entity.get<ComponentShape>());
				writer.op(*entity.get<ComponentMovementParameters>());
			}
		} else {
			ClientRpcProxy::DeleteEntity_ForPeer(realm, peer, entityId);
		}
	}
	icon7::Flags flags = icon7::FLAG_RELIABLE | icon7::FLAGS_CALL_NO_FEEDBACK;
	realm->rpc->FinalizeSerializeSend(writer, flags);
	peer->Send(std::move(writer.Buffer()));
}

void SpawnEntities_ForPeerByIdsVector(RealmServer *realm, icon7::Peer *peer,
									  const std::vector<uint64_t> &ids)
{
	icon7::ByteWriter writer(1000);
	realm->rpc->InitializeSerializeSend(writer,
										ClientRpcFunctionNames::SpawnEntities);
	for (uint64_t entityId : ids) {
		flecs::entity entity = realm->Entity(entityId);
		if (entity.is_alive()) {
			if (entity.has<ComponentLastAuthoritativeMovementState>() &&
				entity.has<ComponentName>() &&
				entity.has<ComponentModelName>() &&
				entity.has<ComponentShape>() &&
				entity.has<ComponentMovementParameters>()) {

				writer.op(entityId);

				writer.op(
					*entity.get<ComponentLastAuthoritativeMovementState>());
				writer.op(*entity.get<ComponentName>());
				writer.op(*entity.get<ComponentModelName>());
				writer.op(*entity.get<ComponentShape>());
				writer.op(*entity.get<ComponentMovementParameters>());
			}
		} else {
			ClientRpcProxy::DeleteEntity_ForPeer(realm, peer, entityId);
		}
	}
	icon7::Flags flags = icon7::FLAG_RELIABLE | icon7::FLAGS_CALL_NO_FEEDBACK;
	realm->rpc->FinalizeSerializeSend(writer, flags);
	peer->Send(std::move(writer.Buffer()));
}

void SpawnPlayerEntity_ForPlayer(RealmServer *realm, icon7::Peer *peer)
{
	icon7::ByteWriter writer(1000);
	realm->rpc->InitializeSerializeSend(writer,
										ClientRpcFunctionNames::SpawnEntities);
	PeerData *data = ((PeerData *)(peer->userPointer));
	uint64_t entityId = data->entityId;
	flecs::entity entity = realm->Entity(entityId);
	if (entity.is_alive()) {
		if (entity.has<ComponentLastAuthoritativeMovementState>() &&
			entity.has<ComponentName>() && entity.has<ComponentModelName>() &&
			entity.has<ComponentShape>() &&
			entity.has<ComponentMovementParameters>()) {

			writer.op(entityId);

			writer.op(*entity.get<ComponentLastAuthoritativeMovementState>());
			writer.op(*entity.get<ComponentName>());
			writer.op(*entity.get<ComponentModelName>());
			writer.op(*entity.get<ComponentShape>());
			writer.op(*entity.get<ComponentMovementParameters>());
		}
	}
	icon7::Flags flags = icon7::FLAG_RELIABLE | icon7::FLAGS_CALL_NO_FEEDBACK;
	realm->rpc->FinalizeSerializeSend(writer, flags);
	peer->Send(std::move(writer.Buffer()));
}

void Broadcast_SetModel(RealmServer *realm, uint64_t entityId,
						const std::string &modelName, ComponentShape shape)
{
	realm->BroadcastReliable(ClientRpcFunctionNames::SetModel, entityId,
							 modelName, shape);
}

void Broadcast_SpawnEntity(RealmServer *realm, uint64_t entityId,
						   const ComponentLastAuthoritativeMovementState &state,
						   const ComponentShape &shape,
						   const ComponentModelName &entityModelName,
						   const ComponentName &entityName,
						   const ComponentMovementParameters &movementParams)
{
	realm->BroadcastReliable(ClientRpcFunctionNames::SpawnEntities, entityId,
							 state, entityName, entityModelName, shape,
							 movementParams);
}

void Broadcast_UpdateEntities(RealmServer *realm)
{
	const static uint32_t singleEntitySize = 8 + 8 + 12 + 12 + 12 + 1;

	int written = 0;

	icon7::ByteWriter writer(1500);

	realm->queryLastAuthoritativeState.each(
		[&](flecs::entity entity,
			const ComponentLastAuthoritativeMovementState &state) {
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

void LoginFailed(icon7::Peer *peer, const std::string &reason)
{
	peer->host->GetRpcEnvironment()->Send(
		peer, icon7::FLAG_RELIABLE | icon7::FLAGS_CALL_NO_FEEDBACK,
		ClientRpcFunctionNames::LoginFailed, reason);
}

void Broadcast_SpawnStaticEntities(RealmServer *realm, uint64_t entityId,
								   const ComponentStaticTransform &transform,
								   const ComponentModelName &model,
								   const ComponentCollisionShape &shape)
{
	if (realm->HasComponent<TagPrivateEntity>(entityId)) {
		return;
	}
	realm->BroadcastReliable(ClientRpcFunctionNames::SpawnStaticEntities,
							 entityId, transform, model, shape);
}

void SpawnStaticEntities_ForPeer(RealmServer *realm, icon7::Peer *peer)
{
	icon7::ByteWriter writer(1500);
	realm->rpc->InitializeSerializeSend(
		writer, ClientRpcFunctionNames::SpawnStaticEntities);
	int written = 0;
	realm->queryStaticEntity.each([&](flecs::entity entity,
									  const ComponentStaticTransform &transform,
									  const ComponentModelName &model,
									  const ComponentCollisionShape &shape) {
		++written;
		writer.op(entity.id());
		writer.op(transform);
		writer.op(model);
		writer.op(shape);
	});
	if (written > 0) {
		icon7::Flags flags =
			icon7::FLAG_RELIABLE | icon7::FLAGS_CALL_NO_FEEDBACK;
		realm->rpc->FinalizeSerializeSend(writer, flags);
		realm->Broadcast(writer.Buffer(), 0);
	}
}

void GenericComponentUpdate_Start(RealmServer *realm, icon7::ByteWriter *writer)
{
	realm->rpc->InitializeSerializeSend(
		*writer, ClientRpcFunctionNames::GenericComponentUpdate);
}

void GenericComponentUpdate_Finish(RealmServer *realm, icon7::Peer *peer,
								   icon7::ByteWriter *writer)
{
	icon7::Flags flags = icon7::FLAG_RELIABLE | icon7::FLAGS_CALL_NO_FEEDBACK;
	realm->rpc->FinalizeSerializeSend(*writer, flags);
	peer->Send(std::move(writer->Buffer()));
}

void Broadcast_PlayDeathAndDestroyEntity(RealmServer *realm, uint64_t entityId)
{
	auto entity = realm->Entity(entityId);
	if (entity.is_valid() && entity.is_alive()) {
		auto modelName = entity.get<ComponentModelName>();
		auto state = entity.get<ComponentLastAuthoritativeMovementState>();
		auto name = entity.get<ComponentName>();
		if (modelName == nullptr || state == nullptr || name == nullptr) {
			LOG_INFO(
				"Entity does not have one of: ModeLname, MovementState, Name");
			return;
		}
		realm->BroadcastReliable(
			ClientRpcFunctionNames::PlayDeathAndDestroyEntity, entityId,
			*modelName, *state, *name);
	} else {
		LOG_WARN("Trying to broadcast death of already dead entity.");
	}
}

void Broadcast_PlayAnimation(RealmServer *realm, uint64_t entityId,
							 ComponentModelName modelName,
							 ComponentLastAuthoritativeMovementState state,
							 std::string currentAnimation,
							 int64_t animationStartTick)
{
	// TODO: move arguments fetching to inside of this function to reduce
	//       arguments count and error proneness
	realm->BroadcastReliable(ClientRpcFunctionNames::PlayAnimation, entityId,
							 modelName, state, currentAnimation);
}

void Broadcast_PlayFX(RealmServer *realm, ComponentModelName modelName,
					  ComponentStaticTransform transform,
					  int64_t timeStartPlaying, uint64_t attachToEntityId,
					  int32_t ttlMs)
{
	// TODO: verify
	realm->BroadcastReliable(ClientRpcFunctionNames::PlayFX, modelName,
							 transform, timeStartPlaying, attachToEntityId);
}
} // namespace ClientRpcProxy
