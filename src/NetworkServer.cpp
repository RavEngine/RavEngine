#include "NetworkServer.hpp"
#include "Debug.hpp"
#include "Entity.hpp"
#include "NetworkReplicable.hpp"
#include "NetworkIdentity.hpp"
#include "World.hpp"
#include <steam/isteamnetworkingutils.h>
#include "App.hpp"
#include "RPCComponent.hpp"

using namespace RavEngine;
using namespace std;
NetworkServer* NetworkServer::currentServer = nullptr;

NetworkServer::NetworkServer() : interface(SteamNetworkingSockets()){}

void NetworkServer::SpawnEntity(Ref<Entity> entity) {
	auto casted = dynamic_pointer_cast<NetworkReplicable>(entity);
	auto world = entity->GetWorld().lock();
	if (casted && world) {
		auto id = casted->NetTypeID();
        auto comp = entity->GetComponent<NetworkIdentity>();
		auto netID = comp->GetNetworkID();
		//send highest-priority safe message with this info to clients
		auto message = CreateSpawnCommand(netID, id, world->worldID);
        NetworkIdentities[netID] = comp;
		for (auto connection : clients) {
			interface->SendMessageToConnection(connection, message.c_str(), message.size(), k_nSteamNetworkingSend_Reliable, nullptr);
		}
	}
	else {
		Debug::Warning("Attempted to spawn entity that is not in a world or is not NetworkReplicable");
	}
}

void NetworkServer::DestroyEntity(Ref<Entity> entity){
	auto casted = dynamic_pointer_cast<NetworkReplicable>(entity);
	if (casted){
		auto netID = entity->GetComponent<NetworkIdentity>()->GetNetworkID();
		auto message = CreateDestroyCommand(netID);
        NetworkIdentities.erase(netID);
		for (auto connection : clients) {
			interface->SendMessageToConnection(connection, message.c_str(), message.size(), k_nSteamNetworkingSend_Reliable, nullptr);
		}
	}
	else {
		Debug::Warning("Attempted to destroy entity that is not NetworkReplicable");
	}
}

void RavEngine::NetworkServer::SendMessageToAllClients(const std::string& msg) const
{
	for (const auto connection : clients) {
		interface->SendMessageToConnection(connection, msg.data(), msg.length(), k_nSteamNetworkingSend_Reliable, nullptr);
	}
}

void NetworkServer::Start(uint16_t port){
	//configure and start server
	SteamNetworkingConfigValue_t opt;
	opt.SetPtr(k_ESteamNetworkingConfig_ConnectionUserData, (void*)this);	//the thisptr
	opt.SetPtr(k_ESteamNetworkingConfig_Callback_ConnectionStatusChanged, (void*)NetworkServer::SteamNetConnectionStatusChanged);
	SteamNetworkingIPAddr serverLocalAddr;
	serverLocalAddr.Clear();
	serverLocalAddr.m_port = port;
	listenSocket = interface->CreateListenSocketIP(serverLocalAddr, 1, &opt);
	
	if (listenSocket == k_HSteamListenSocket_Invalid )
		Debug::Fatal( "Failed to listen on port {}", port );
	
	pollGroup = interface->CreatePollGroup();
	if (pollGroup == k_HSteamNetPollGroup_Invalid)
		Debug::Fatal("Failed to create poll group");
	
	Debug::Log("Listening on port {}",port);
	
	currentServer = this;
	
	workerIsRunning = true;
	worker = std::thread(&NetworkServer::ServerTick,this);
	worker.detach();
}

void NetworkServer::Stop(){
	workerIsRunning = false;	//this unblocks the worker thread, allowing it to exit
	while(!workerHasStopped);	//wait for thread to exit
	interface->CloseListenSocket(listenSocket);
	interface->DestroyPollGroup(pollGroup);
}

NetworkServer::~NetworkServer(){
	Stop();
}

void NetworkServer::OnSteamNetConnectionStatusChanged(SteamNetConnectionStatusChangedCallback_t *pInfo){
	// What's the state of the connection?
	switch ( pInfo->m_info.m_eState ){
		
		case k_ESteamNetworkingConnectionState_None:
			// NOTE: We will get callbacks here when we destroy connections.  You can ignore these.
			break;
		case k_ESteamNetworkingConnectionState_Connecting:
			// This must be a new connection
			assert( !clients.contains(pInfo->m_hConn) );
			
			// A client is attempting to connect
			// Try to accept the connection.
			if ( interface->AcceptConnection( pInfo->m_hConn ) != k_EResultOK )
			{
				// This could fail.  If the remote host tried to connect, but then
				// disconnected, the connection may already be half closed.  Just
				// destroy whatever we have on our side.
				interface->CloseConnection( pInfo->m_hConn, 0, nullptr, false );
				Debug::Error("Can't accept connection, was it already closed?");
				break;
			}
			
			// Assign the poll group
			if ( !interface->SetConnectionPollGroup( pInfo->m_hConn, pollGroup ) )
			{
				interface->CloseConnection( pInfo->m_hConn, 0, nullptr, false );
				Debug::Error( "Failed to set poll group" );
				break;
			}
			
			//track the connection
			clients.insert(pInfo->m_hConn);
			
			break;
		case k_ESteamNetworkingConnectionState_FindingRoute:
			
			break;
		case k_ESteamNetworkingConnectionState_Connected:
			// We will get a callback immediately after accepting the connection.
			// Since we are the server, we can ignore this, it's not news to us.
			break;
		case k_ESteamNetworkingConnectionState_ClosedByPeer:
		case k_ESteamNetworkingConnectionState_ProblemDetectedLocally:
			// Ignore if they were not previously connected.  (If they disconnected
			// before we accepted the connection.)
			if ( pInfo->m_eOldState == k_ESteamNetworkingConnectionState_Connected )
			{
				// Locate the client.  Note that it should have been found, because this
				// is the only codepath where we remove clients (except on shutdown),
				// and connection change callbacks are dispatched in queue order.
				assert(clients.contains(pInfo->m_hConn));
				
				// Select appropriate log messages
				const char *pszDebugLogAction;
				if ( pInfo->m_info.m_eState == k_ESteamNetworkingConnectionState_ProblemDetectedLocally )
				{
					Debug::Warning("Networking problem detected locally");
				}
				else
				{
					// Note that here we could check the reason code to see if
					// it was a "usual" connection or an "unusual" one.
					Debug::Warning("Networking closed by peer");
				}
				
				clients.erase(pInfo->m_hConn);
			}
			else{
				assert( pInfo->m_eOldState == k_ESteamNetworkingConnectionState_Connecting );
			}
			
			// Clean up the connection.  This is important!
			// The connection is "closed" in the network sense, but
			// it has not been destroyed.  We must close it on our end, too
			// to finish up.  The reason information do not matter in this case,
			// and we cannot linger because it's already closed on the other end,
			// so we just pass 0's.
			interface->CloseConnection( pInfo->m_hConn, 0, nullptr, false );
			
			break;
		case k_ESteamNetworkingConnectionState_FinWait:
			
			break;
		case k_ESteamNetworkingConnectionState_Linger:
			
			break;
		case k_ESteamNetworkingConnectionState_Dead:
			
			break;
		case k_ESteamNetworkingConnectionState__Force32Bit:
			
			break;
	}
}

void NetworkServer::SteamNetConnectionStatusChanged(SteamNetConnectionStatusChangedCallback_t * pInfo){
	currentServer->OnSteamNetConnectionStatusChanged(pInfo);
}

void NetworkServer::ServerTick(){
	while(workerIsRunning){
		
		//get incoming messages
		while (workerIsRunning){	//do we need this double loop?
			ISteamNetworkingMessage *pIncomingMsg = nullptr;
			int numMsgs = interface->ReceiveMessagesOnPollGroup( pollGroup, &pIncomingMsg, 1 );
			if ( numMsgs == 0 ){
				break;
			}
			if ( numMsgs < 0 ){
				Debug::Fatal( "Error checking for messages" );
			}
			
			//is this from a connected client
			assert(clients.contains(pIncomingMsg->m_conn));
			
			//figure out what to do with the message
            //get the command code (first byte in the message)
            std::string_view message((char*)pIncomingMsg->m_pData, pIncomingMsg->m_cbSize);
            uint8_t cmdcode = message[0];
            switch (cmdcode) {
            case NetworkBase::CommandCode::RPC:
                //TODO: server needs to check ownership, client does not
				OnRPC(message);
                break;
            default:
                Debug::Warning("Invalid command code: {}",cmdcode);
            }
			
			//deallocate when done
			pIncomingMsg->Release();
		}
				
		//invoke callbacks
		interface->RunCallbacks();
	}
	workerHasStopped = true;
}

void RavEngine::NetworkServer::OnRPC(const std::string_view& cmd)
{
	//decode the RPC header to to know where it is going

	uuids::uuid id(cmd.data() + 1);
	if (NetworkIdentities.contains(id)) {
		NetworkIdentities.at(id)->getOwner().lock()->GetComponent<RPCComponent>()->CacheServerRPC(cmd);
	}

}