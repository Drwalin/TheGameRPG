#include <icon7/Peer.hpp>

#include "../../common/include/GlmSerialization.hpp"

#include "../include/RealmServer.hpp"

#include "../include/EntityServer.hpp"

EntityServer::EntityServer()
{
}

EntityServer::~EntityServer()
{
	DisconnectPeer();
}

void EntityServer::ConnectPeer(icon7::Peer *peer)
{
	PeerData *data = (PeerData *)(peer->userPointer);
	if (data->realm) {
		DEBUG("Error: Entity::ConnectPeer cannot be executed before "
			  "disconnecting from old realm.");
	}

	data->entityId = entityId;
	longState.userName = data->userName;
	this->peer = peer->shared_from_this();
	// TODO: load entity data from database
}

void EntityServer::DisconnectPeer()
{
	if (peer) {
		// TODO: safe entity data to database
		// TODO: remove entity from system ??
		((PeerData *)(peer->userPointer))->realm = nullptr;
		((PeerData *)(peer->userPointer))->entityId = 0;
		peer = nullptr;
	}
}

void EntityServer::SetModel(const std::string &modelName, float height, float width)
{
	EntityBase::SetModel(modelName, height, width);
	Realm()->BroadcastEntityChangeModel(entityId);
}

