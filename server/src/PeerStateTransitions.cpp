#include "../include/PeerData.hpp"
#include "../include/RealmServer.hpp"
#include "../include/ServerCore.hpp"
#include "../include/DBWorker.hpp"
#include "../include/ClientRpcProxy.hpp"

#include "../include/PeerStateTransitions.hpp"

namespace peer_transitions
{
void OnReceivedLogin(icon7::Peer *peer, const std::string &username)
{
	PeerData *data = ((PeerData *)(peer->userPointer));
	if (data->userName != "" || username == "" || data->peer.expired()) {
		// TODO: re-login is not available
		LOG_INFO("Login failed: '%s' != ""  ||  '%s' == ""  ||  %s == true",
				data->userName.c_str(), username.c_str(),
				data->peer.expired() ? "true" : "false"
				);
		ClientRpcProxy::LoginFailed(peer);
		return;
	}

	DBWorker::GetSingleton()->EnqueueCommand(
		+[](std::weak_ptr<icon7::Peer> _peer, std::string username) {
			auto peer = _peer.lock();
			if (!peer) {
				// peer already disconnected
				return;
			}
			auto &db = *DBWorker::GetSingleton();
			auto core = ((ServerCore *)(peer->host->userPointer));

			static sqlite::Statement stmt = db.PrepareStatement(
				"SELECT username, realm_name FROM Users WHERE ?;",
				(int *)nullptr);

			stmt.BindString(1, username);

			int rc = stmt.Step();
			if (rc == sqlite::ROW) {
				// TODO: user exists in database, perform join to realm
				std::string_view sv = stmt.ColumnString(2);
				std::string realmName{sv.data(), sv.size()};
// 				core->ConnectPeerToRealm(peer.get(), realmName);
				
				
				
			} else if (rc == sqlite::DONE) {
				// TODO: no user exists in database, spawn user
				
				
				
			} else {
				LOG_ERROR("Failed to execute user select from database: %s",
						  db.ErrorMessage().c_str());
			}
			
			
			
			stmt.Reset();
			stmt.ClearBindings();
		}, peer->weak_from_this(), username);
	
	
	
	auto core = ((ServerCore *)(peer->host->userPointer));
	// TODO: get player data from database and call core->ConnectPeerToRealm
	core->ConnectPeerToRealm(peer, core->spawnRealm);
}
} // namespace peer_transitions
