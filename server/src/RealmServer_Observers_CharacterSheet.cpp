#define ENABLE_REALM_SERVER_IMPLEMENTATION_TEMPLATE

#include "../../common/include/ComponentCharacterSheet.hpp"

#include "../include/ClientRpcProxy.hpp"
#include "../include/EntityNetworkingSystems.hpp"

#include "../include/RealmServer.hpp"

#define REGISTER_OBSERVER_TO_SEND_COMPONENT_TO_OWNING_PEER(CLASS)              \
	RegisterObserver(                                                          \
		flecs::OnSet, [this](flecs::entity entity, const CLASS &component,     \
							 const ComponentPlayerConnectionPeer &peer) {      \
			icon7::ByteWriter writer(256);                                     \
			ClientRpcProxy::GenericComponentUpdate_Start(this, &writer);       \
			ClientRpcProxy::GenericComponentUpdate_Update<CLASS>(              \
				this, &writer, entity, component);                             \
			ClientRpcProxy::GenericComponentUpdate_Finish(                     \
				this, peer.peer.get(), &writer);                               \
		})

void RealmServer::RegisterObservers_CharacterSheet()
{
	REGISTER_OBSERVER_TO_SEND_COMPONENT_TO_OWNING_PEER(
		ComponentCharacterSheet_Ranges);
	REGISTER_OBSERVER_TO_SEND_COMPONENT_TO_OWNING_PEER(
		ComponentCharacterSheet_Health);
	REGISTER_OBSERVER_TO_SEND_COMPONENT_TO_OWNING_PEER(
		ComponentCharacterSheet_HealthRegen);
	REGISTER_OBSERVER_TO_SEND_COMPONENT_TO_OWNING_PEER(
		ComponentCharacterSheet_LevelXP);
	REGISTER_OBSERVER_TO_SEND_COMPONENT_TO_OWNING_PEER(
		ComponentCharacterSheet_Strength);
	REGISTER_OBSERVER_TO_SEND_COMPONENT_TO_OWNING_PEER(
		ComponentCharacterSheet_AttackCooldown);
	REGISTER_OBSERVER_TO_SEND_COMPONENT_TO_OWNING_PEER(
		ComponentCharacterSheet_Protection);
}
