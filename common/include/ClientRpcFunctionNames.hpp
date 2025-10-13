#pragma once

#include <string>

namespace ClientRpcFunctionNames
{
// void JoinRealm(string, Tick currentTick, uint64_t playerId)
inline const std::string JoinRealm = "JoinRealm";

// void SpawnStaticEntities({entityId, StaticTransform, ModelName,
//                           StaticCollisionShapeName}, ...)
inline const std::string SpawnStaticEntities = "SpawnStaticEntities";

// void SpawnEntities({entityId, LastAuthoritativeMovementState, EntityName,
//                     ModelName,
//                     EntityShape, MovementParams}, ...)
inline const std::string SpawnEntities = "SpawnEntities";

// void UpdateEntities(Tick tick, {entityId, LastAuthoritativeMovementState}, ...)
inline const std::string UpdateEntities = "UpdateEntities";

// void SetModel(entityId, ModelName, EntityShape)
inline const std::string SetModel = "SetModel";

// void DeleteEntities({entityId}, ...)
inline const std::string DeleteEntities = "DeleteEntities";

// void SetPlayerEntityId(uint64_t)
inline const std::string SetPlayerEntityId = "SetPlayerEntityId";

// void SetGravity(float)
inline const std::string SetGravity = "SetGravity";

// void LoginFailed(string reason)
inline const std::string LoginFailed = "LoginFailed";

// void LoginSuccessfull()
inline const std::string LoginSuccessfull = "LoginSuccessfull";

// void Pong(Tick clientLastSentTick, int64_t clientLastSentTickTimeNs,
//           Tick serverLastProcessedTick,
//           int64_t serverTickStartTimeOffsetNs,
//           int64_t clientPingSentTimeNs)
inline const std::string Pong = "Pong";

// void GenericEntityComponentUpdate({entityId, string componentName,
// 									  componentData}, ...)
inline const std::string GenericComponentUpdate = "GenericComponentUpdate";

// void PlayDeathAndDestroyEntity(entityId, string ModelName,
// 								  LastAuthoritativeMovementState, EntityName)
inline const std::string PlayDeathAndDestroyEntity =
	"PlayDeathAndDestroyEntity";

// void PlayAnimation(entityId, ModelName, LastAuthoritativeMovementState,
// 					  string currentAnimation, Tick animationStartTick)
inline const std::string PlayAnimation = "PlayAnimation";

// void PlayFX(ModelName, StaticTransform, int64_t timeStartPlaying,
// 			   attachToEntityId, int32_t ttlMs)
inline const std::string PlayFX = "PlayFX";
} // namespace ClientRpcFunctionNames
