#pragma once

#include <string>

namespace ClientRpcFunctionNames
{
// void JoinRealm(string, int64_t currentTick, uint64_t playerId)
inline const std::string JoinRealm = "JoinRealm";

// void SpawnStaticEntities({entityId, StaticTransform, ModelName,
//                           StaticCollisionShapeName}, ...)
inline const std::string SpawnStaticEntities = "SpawnStaticEntities";

// void SpawnEntities({entityId, MovementState, EntityName, ModelName,
//                     EntityShape, MovementParams}, ...)
inline const std::string SpawnEntities = "SpawnEntities";

// void UpdateEntities({entityId, MovementState}, ...)
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

// void Pong(int64_t internal, int64_t serverMostCurrentTick)
inline const std::string Pong = "Pong";

// void GenericEntityComponentUpdate({entityId, string componentName,
// 									  componentData}, ...)
inline const std::string GenericComponentUpdate = "GenericComponentUpdate";

// void PlayDeathAndDestroyEntity(entityId, string ModelName,
// 								  MovementState, EntityName)
inline const std::string PlayDeathAndDestroyEntity =
	"PlayDeathAndDestroyEntity";

// void PlayAnimation(entityId, ModelName, MovementState,
// 					  string currentAnimation, int64_t animationStartTick)
inline const std::string PlayAnimation = "PlayAnimation";

// void PlayFX(ModelName, StaticTransform, int64_t timeStartPlaying,
// 			   attachToEntityId)
inline const std::string PlayFX = "PlayFX";
} // namespace ClientRpcFunctionNames
