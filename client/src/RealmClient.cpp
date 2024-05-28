#include "../../common/include/CollisionLoader.hpp"

#include "../include/GameClient.hpp"

#include "../include/RealmClient.hpp"

RealmClient::RealmClient(GameClient *gameClient) : gameClient(gameClient)
{
	this->minDeltaTicks = 3;
	this->maxDeltaTicks = 9;
	RealmClient::RegisterSystems();
	RealmClient::RegisterObservers();
}

RealmClient::~RealmClient() { peer = nullptr; }

void RealmClient::Init(const std::string &realmName)
{
	// TODO: load static realm data from database/disk
	Realm::Init(realmName);
	
	CollisionLoader loader;
	loader.LoadOBJ(std::string("assets/models/") + realmName + ".obj");
	collisionWorld.LoadStaticCollision(&loader.collisionData);
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
						 gameClient->OnEntityAdd(entity.id());
					 });

	RegisterObserver(flecs::OnRemove,
					 [this](flecs::entity entity, const EntityMovementState &) {
						 gameClient->OnEntityRemove(entity.id());
					 });

	RegisterObserver(flecs::OnSet, [this](flecs::entity entity,
										  const EntityModelName &model) {
		gameClient->OnEntityModel(entity.id(), model);
	});

	RegisterObserver(flecs::OnSet,
					 [this](flecs::entity entity, const EntityShape &shape) {
						 gameClient->OnEntityShape(entity.id(), shape);
					 });
}

void RealmClient::RegisterSystems() { Realm::RegisterSystems(); }
