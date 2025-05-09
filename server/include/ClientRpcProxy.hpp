#pragma once

#include <icon7/Forward.hpp>
#include <icon7/Flags.hpp>
#include <icon7/ByteWriter.hpp>

#include "../../common/include/RegistryComponent.hpp"
#include "../../common/include/EntityComponents.hpp"

class RealmServer;

namespace ClientRpcProxy
{
void JoinRealm(RealmServer *realm, icon7::Peer *peer, uint64_t playerEntityId);

void SetPlayerEntityId(RealmServer *realm, icon7::Peer *peer,
					   uint64_t playerEntityId);
void Pong(icon7::Peer *peer, icon7::Flags flags, int64_t data);
void SetGravity(RealmServer *realm, icon7::Peer *peer, float gravity);

void DeleteEntity_ForPeer(RealmServer *realm, icon7::Peer *peer,
						  uint64_t entityId);

void SpawnEntities_ForPeer(RealmServer *realm, icon7::Peer *peer);
void SpawnEntities_ForPeerByIds(RealmServer *realm, icon7::Peer *peer,
								icon7::ByteReader &reader);
void SpawnEntities_ForPeerByIdsVector(RealmServer *realm, icon7::Peer *peer,
									  const std::vector<uint64_t> &ids);
void SpawnPlayerEntity_ForPlayer(RealmServer *realm, icon7::Peer *peer);

void Broadcast_SetModel(RealmServer *realm, uint64_t entityId,
						const std::string &modelName, ComponentShape shape);
void Broadcast_SpawnEntity(RealmServer *realm, uint64_t entityId,
						   const ComponentLastAuthoritativeMovementState &state,
						   const ComponentShape &shape,
						   const ComponentModelName &entityModelName,
						   const ComponentName &entityName,
						   const ComponentMovementParameters &movementParams);
void Broadcast_UpdateEntities(RealmServer *realm);
void Broadcast_DeleteEntity(RealmServer *realm, uint64_t entityId);

void LoginSuccessfull(icon7::Peer *peer);
void LoginFailed(icon7::Peer *peer, const std::string &reason);

void Broadcast_SpawnStaticEntities(
	RealmServer *realm, uint64_t entityId,
	const ComponentStaticTransform &transform, const ComponentModelName &model,
	const ComponentCollisionShape &shape);
void SpawnStaticEntities_ForPeer(RealmServer *realm, icon7::Peer *peer);

void GenericComponentUpdate_Start(RealmServer *realm,
								  icon7::ByteWriter *writer);
template <typename... TArgs>
void GenericComponentUpdate_Update(RealmServer *realm,
								   icon7::ByteWriter *writer, uint64_t entityId)
{
	writer->op(entityId);
	(reg::Registry::SerializeTemporal<TArgs>(realm, entityId, *writer), ...);
	writer->op("");
}

template <typename... TArgs>
void GenericComponentUpdate_Update(RealmServer *realm,
								   icon7::ByteWriter *writer, uint64_t entityId,
								   const TArgs &...components)
{
	writer->op(entityId);
	(reg::Registry::SerializeTemporal<TArgs>(realm, components, *writer), ...);
	writer->op("");
}
void GenericComponentUpdate_Finish(RealmServer *realm, icon7::Peer *peer,
								   icon7::ByteWriter *writer);

void Broadcast_PlayDeathAndDestroyEntity(RealmServer *realm, uint64_t entityId);
void Broadcast_PlayAnimation(RealmServer *realm, uint64_t entityId,
							 ComponentModelName modelName,
							 ComponentLastAuthoritativeMovementState state,
							 std::string currentAnimation,
							 int64_t animationStartTick);
void Broadcast_PlayFX(RealmServer *realm, ComponentModelName modelName,
					  ComponentStaticTransform transform,
					  int64_t timeStartPlaying, uint64_t attachToEntityId,
					  int32_t ttlMs);
} // namespace ClientRpcProxy
