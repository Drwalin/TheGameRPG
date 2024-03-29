#include "../include/ClientCore.hpp"

#include "../include/RealmClient.hpp"

RealmClient::RealmClient()
{
	this->minDeltaTicks = 3;
	this->maxDeltaTicks = 16;
}

RealmClient::~RealmClient() { peer = nullptr; }

void RealmClient::Init(const std::string &realmName)
{
	// TODO: load static realm data from database/disk
	Realm::Init(realmName);
}

void RealmClient::Clear()
{
	Realm::Clear();
	peer = nullptr;
}

void RealmClient::Reinit(const std::string &realmName)
{
	Clear();
	Init(realmName);
}

bool RealmClient::OneEpoch()
{
	Realm::OneEpoch();

	return true;
}

void RealmClient::RegisterObservers()
{
	Realm::RegisterObservers();

	RegisterObserver(flecs::OnAdd,
					 [this](flecs::entity entity, const EntityMovementState &,
							const EntityLastAuthoritativeMovementState &,
							const EntityName, const EntityModelName,
							const EntityShape, const EntityName &name) {
						 clientCore->OnEntityAdd(entity.id());
					 });

	RegisterObserver(flecs::OnRemove,
					 [this](flecs::entity entity, const EntityMovementState &) {
						 clientCore->OnEntityRemove(entity.id());
					 });

	RegisterObserver(flecs::OnSet, [this](flecs::entity entity,
										  const EntityModelName &model) {
		clientCore->OnEntityModel(entity.id(), model);
	});

	RegisterObserver(flecs::OnSet,
					 [this](flecs::entity entity, const EntityShape &shape) {
						 clientCore->OnEntityShape(entity.id(), shape);
					 });
}

void RealmClient::RegisterSystems()
{
	Realm::RegisterSystems();
	systemsRunPeriodicallyByTimer.push_back(
		ecs.system<const EntityShape, EntityMovementState,
				   const EntityLastAuthoritativeMovementState,
				   const EntityMovementParameters>("UpdateEntityCurrentState")
			.each(
				[this](flecs::entity entity, const EntityMovementState &state) {
					clientCore->OnEntityCurrentMovementStateUpdate(entity.id(),
																   state);
				}));
}
