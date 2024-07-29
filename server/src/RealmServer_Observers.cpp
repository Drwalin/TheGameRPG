#include "../../common/include/EntitySystems.hpp"

#define ENABLE_REALM_SERVER_IMPLEMENTATION_TEMPLATE

#include "../../common/include/ClientRpcFunctionNames.hpp"

#include "../include/ClientRpcProxy.hpp"
#include "../include/EntityNetworkingSystems.hpp"
#include "../include/EntityGameComponents.hpp"
#include "../include/callbacks/CallbackAiBehaviorTick.hpp"
#include "../include/EntityGameComponents.hpp"

#include "../include/RealmServer.hpp"

void RealmServer::RegisterObservers()
{
	EntityNetworkingSystems::RegisterObservers(this);
	RegisterObservers_CharacterSheet();

	queryLastAuthoritativeState =
		ecs.query<const ComponentLastAuthoritativeMovementState>();

	queryEntityLongState =
		ecs.query<const ComponentLastAuthoritativeMovementState,
				  const ComponentName, const ComponentModelName,
				  const ComponentShape, const ComponentMovementParameters>();

	queryStaticEntity =
		ecs.query<const ComponentStaticTransform, const ComponentModelName,
				  const ComponentStaticCollisionShapeName>();

	ecs.observer<ComponentLastAuthoritativeMovementState>()
		.event(flecs::OnSet)
		.each([this](flecs::entity entity,
					 const ComponentLastAuthoritativeMovementState &lastState) {
			BroadcastReliable(ClientRpcFunctionNames::UpdateEntities,
							  entity.id(), lastState);
		});

	ecs.observer<ComponentModelName, ComponentShape>()
		.event(flecs::OnSet)
		.each([this](flecs::entity entity, const ComponentModelName &model,
					 const ComponentShape &shape) {
			ClientRpcProxy::Broadcast_SetModel(this, entity.id(),
											   model.modelName, shape);
		});

	ecs.observer<ComponentStaticTransform, ComponentModelName,
				 ComponentStaticCollisionShapeName>()
		.event(flecs::OnSet)
		.each([this](flecs::entity entity,
					 const ComponentStaticTransform &transform,
					 const ComponentModelName &model,
					 const ComponentStaticCollisionShapeName &shape) {
			// TODO: separate transform broadcast from model and shape broadcast
			ClientRpcProxy::Broadcast_SpawnStaticEntities(
				this, entity, transform, model, shape);
		});

	ecs.observer<const ComponentStaticTransform, ComponentTrigger>()
		.event(flecs::OnAdd)
		.each([this](flecs::entity entity,
					 const ComponentStaticTransform &transform,
					 ComponentTrigger &trigger) {
			collisionWorld.OnAddTrigger(entity, transform);
		});

	ecs.observer<ComponentTrigger>()
		.event(flecs::OnSet)
		.each([this](flecs::entity entity, ComponentTrigger &trigger) {
			trigger.tickUntilIgnore = timer.currentTick + 1;
		});

	ecs.observer<ComponentEventsQueue, const ComponentAITick>()
		.event(flecs::OnAdd)
		.each([this](flecs::entity entity, ComponentEventsQueue &eventsQueue,
					 const ComponentAITick &) {
			static EntityEventTemplate defaultAiMovementEvent{
				[](Realm *realm, int64_t scheduledTick, int64_t currentTick,
				   uint64_t entityId) {
					flecs::entity entity = realm->Entity(entityId);
					if (entity.has<ComponentAITick>()) {
						auto tick = entity.get<ComponentAITick>();
						if (tick) {
							if (tick->aiTick) {
								tick->aiTick->Call((RealmServer *)realm,
												   entityId);
							}
						}
					}

					int64_t dt = realm->maxMovementDeltaTicks;

					EntityEvent event;
					event.dueTick = realm->timer.currentTick + dt;
					event.event = &defaultAiMovementEvent;

					ComponentEventsQueue *eventsQueue =
						realm->AccessComponent<ComponentEventsQueue>(entityId);
					if (eventsQueue == nullptr) {
						LOG_FATAL("Events queue removed but event AIUpdate was "
								  "executed.");
						return;
					}

					eventsQueue->ScheduleEvent(realm, entityId, event);
				}};
			EntityEvent event;
			event.dueTick = timer.currentTick + 100;
			event.event = &defaultAiMovementEvent;
			eventsQueue.ScheduleEvent(this, entity.id(), event);
		});

	ecs.observer<ComponentLastAuthoritativeMovementState>()
		.event(flecs::OnAdd)
		.each([](flecs::entity entity,
				 const ComponentLastAuthoritativeMovementState &) {
			if (entity.has<ComponentEventsQueue>() == false) {
				entity.add<ComponentEventsQueue>();
			}
		});
	ecs.observer<ComponentMovementParameters>()
		.event(flecs::OnAdd)
		.each([](flecs::entity entity, const ComponentMovementParameters &) {
			if (entity.has<ComponentEventsQueue>() == false) {
				entity.add<ComponentEventsQueue>();
			}
		});
	ecs.observer<ComponentMovementState>()
		.event(flecs::OnAdd)
		.each([](flecs::entity entity, const ComponentMovementState &) {
			if (entity.has<ComponentEventsQueue>() == false) {
				entity.add<ComponentEventsQueue>();
			}
		});

	ecs.observer<ComponentAITick>()
		.event(flecs::OnAdd)
		.each([](flecs::entity entity, const ComponentAITick &) {
			if (entity.has<ComponentEventsQueue>() == false) {
				entity.add<ComponentEventsQueue>();
			}
		});

	ecs.observer<ComponentLastAuthoritativeMovementState,
				 ComponentPlayerConnectionPeer>()
		.event(flecs::OnSet)
		.each([this](flecs::entity entity,
					 const ComponentLastAuthoritativeMovementState &state,
					 ComponentPlayerConnectionPeer &peer) {
			if (currentlyUpdatingPlayerPeerEntityMovement == false) {
				if (peer.peer.get() != nullptr) {
					ClientRpcProxy::SpawnEntities_ForPeerByIdsVector(
						this, peer.peer.get(), {entity.id()});
				}
			}
		});
}
