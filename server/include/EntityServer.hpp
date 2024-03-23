#pragma once

#include "PeerData.hpp"
#include "../../ICon7-godot-client/ICon7/include/icon7/Debug.hpp"

#include "../../common/include/EntityBase.hpp"

namespace icon7
{
class Peer;
}

class RealmServer;

class EntityServer : public EntityBase
{
public:
	EntityServer();
	virtual ~EntityServer() override;
	
	void ConnectPeer(icon7::Peer *peer);
	void DisconnectPeer();
	
	virtual void SetModel(const std::string &modelName, float height, float width) override;
	
	std::shared_ptr<icon7::Peer> peer;
	
	inline RealmServer *Realm() { return (RealmServer *)realm; }
};
