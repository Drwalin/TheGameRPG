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

	RegisterObserver(flecs::OnAdd, [](flecs::entity entity, TagPlayerEntity) {
		if (entity.has<TagNonPlayerEntity>()) {
			entity.remove<TagNonPlayerEntity>();
		}
	});

	RegisterObserver(
		flecs::OnAdd, [](flecs::entity entity, TagNonPlayerEntity) {
			if (entity.has<TagPlayerEntity>()) {
				LOG_FATAL("Entity has added TagNonPlayerEntity while having "
						  "TagPlayerEntity component.");
				entity.remove<TagNonPlayerEntity>();
			}
		});

	queryLastAuthoritativeState =
		ecs.query<const ComponentLastAuthoritativeMovementState>();

	queryEntityLongState =
		ecs.query<const ComponentLastAuthoritativeMovementState,
				  const ComponentName, const ComponentModelName,
				  const ComponentShape, const ComponentMovementParameters>();

	queryStaticEntity =
		ecs.query<const ComponentStaticTransform, const ComponentModelName,
				  const ComponentCollisionShape>();

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
				 ComponentCollisionShape>()
		.event(flecs::OnSet)
		.each([this](flecs::entity entity,
					 const ComponentStaticTransform &transform,
					 const ComponentModelName &model,
					 const ComponentCollisionShape &shape) {
			// TODO: separate transform broadcast from model and shape broadcast
			ClientRpcProxy::Broadcast_SpawnStaticEntities(
				this, entity, transform, model, shape);
		});

	ecs.observer<ComponentTrigger>()
		.event(flecs::OnAdd)
		.each([this](flecs::entity entity, ComponentTrigger &trigger) {
			entity.add<TagPrivateEntity>();
			ClientRpcProxy::Broadcast_DeleteEntity(this, entity.id());
		});

	ecs.observer<ComponentTrigger>()
		.event(flecs::OnSet)
		.each(+[](flecs::entity entity, const ComponentTrigger &) {
			if (auto shape = entity.try_get_mut<ComponentCollisionShape>()) {
				shape->mask = FILTER_TRIGGER;
				ComponentCollisionShape s = *shape;
				entity.set(s);
				LOG_INFO("Setting trigger shape filter");
			}
		});

	ecs.observer<ComponentCollisionShape>()
		.event(flecs::OnSet)
		.each(+[](flecs::entity entity, ComponentCollisionShape shape) {
			if (entity.try_get<ComponentTrigger>()) {
				if (shape.mask & FILTER_TRIGGER) {
					return;
				}
				shape.mask = FILTER_TRIGGER;
				entity.set(shape);
				LOG_INFO("Setting trigger shape filter");
			}
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
				"defaultAiMovementEvent",
				+[](Realm *realm, Tick scheduledTick, Tick currentTick,
					uint64_t entityId) -> Tick {
					flecs::entity entity = realm->Entity(entityId);
					if (entity.has<ComponentAITick>()) {
						auto tick = entity.try_get<ComponentAITick>();
						if (tick) {
							if (tick->aiTick) {
								tick->aiTick->Call((RealmServer *)realm,
												   entityId);
							}
						}
					}

					return {3};
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

	RegisterObserver(
		flecs::OnAdd, +[](flecs::entity entity, ComponentSpawner &) {
			if (entity.has<ComponentEventsQueue>() == false) {
				entity.add<ComponentEventsQueue>();
			}
		});
}
