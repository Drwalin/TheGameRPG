#include <icon7/Peer.hpp>
#include <icon7/Flags.hpp>
#include <icon7/Debug.hpp>

#include "../../common/include/EntitySystems.hpp"

#include "../include/ClientRpcProxy.hpp"
#include "../include/EntityNetworkingSystems.hpp"
#include "../include/EntityGameComponents.hpp"
#include "../include/callbacks/CallbackOnUse.hpp"

#include "../include/RealmServer.hpp"

void RealmServer::InteractInLineOfSight(uint64_t instigatorId,
										ComponentMovementState state,
										uint64_t targetId, glm::vec3 dstPos,
										glm::vec3 normal)
{
	/*
	glm::vec3 pos = state.pos;
	glm::vec3 rot = state.rot;
	// TODO: replace 4 with value from some entity component
	glm::vec3 forward = glm::vec3(0, 0, 4);

	glm::mat4 mat(1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1);
	mat = glm::rotate(mat, rot.y, glm::vec3(0, 1, 0));
	mat = glm::rotate(mat, rot.x, glm::vec3(-1, 0, 0));
	glm::vec4 v4 = {forward.x, forward.y, forward.z, 0};
	forward = mat * v4;

	flecs::entity entity = Entity(instigatorId);
	const auto shape = entity.get<ComponentShape>();
	if (shape == nullptr) {
		// TODO: maybe error?
		return;
	}

	pos.y += shape->height;

	glm::vec3 dst = pos + forward;

	glm::vec3 hitPoint, normal;
	float td;
	bool hasNormal;
	uint64_t entityId = 0;
	if (false == collisionWorld.RayTestFirstHit(pos, dst, &hitPoint, &normal,
											  &entityId, &td, &hasNormal,
											  instigatorId)) {
		// TODO: Show no interaction
		return;
	}
	*/

	flecs::entity srcEntity = Entity(instigatorId);
	flecs::entity targetEntity = Entity(targetId);
	if (srcEntity.is_valid() && targetEntity.is_valid() &&
		srcEntity.is_alive() && targetEntity.is_alive()) {
		if (targetEntity.has<ComponentOnUse>()) {
			const ComponentOnUse *onUse = targetEntity.get<ComponentOnUse>();
			if (onUse && onUse->entry) {
				onUse->entry->Call(this, instigatorId, targetId, "");
			}
		}
	}
}

void RealmServer::InteractInLineOfSight(icon7::Peer *peer,
										ComponentMovementState state,
										uint64_t targetId, glm::vec3 dstPos,
										glm::vec3 normal)
{
	PeerData *data = ((PeerData *)(peer->userPointer));
	InteractInLineOfSight(data->entityId, state, targetId, dstPos, normal);
}
