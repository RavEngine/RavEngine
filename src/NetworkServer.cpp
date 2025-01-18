#include "NetworkServer.hpp"
#include "Debug.hpp"
#include "Entity.hpp"
#include "NetworkIdentity.hpp"
#include "World.hpp"
#include <steam/isteamnetworkingutils.h>
#include "App.hpp"
#include "RPCComponent.hpp"

using namespace RavEngine;
using namespace std;
NetworkServer* NetworkServer::currentServer = nullptr;

NetworkServer::NetworkServer() : net_interface(SteamNetworkingSockets()){}

void RavEngine::NetworkServer::HandleDisconnect(HSteamNetConnection connection)
{
	for (auto entity : OwnershipTracker[connection]) {
        entity.GetOwner().Destroy();
    }
	OwnershipTracker.erase(connection);
}

void NetworkServer::SpawnEntity(World* source, ctti_t id, Entity ent_id, const uuids::uuid& netID) {
	NetworkIdentities[netID] = ent_id;
    auto message = CreateSpawnCommand(netID,id,source->worldID);
	auto len = message.size();
	assert(len < numeric_limits<uint32_t>::max());	// message is too long!
    for (auto connection : clients) {
        net_interface->SendMessageToConnection(connection, message.c_str(), static_cast<uint32_t>(len), k_nSteamNetworkingSend_Reliable, nullptr);
    }
}

void NetworkServer::DestroyEntity(const uuids::uuid& netID){
	NetworkIdentities.erase(netID);
    auto message = CreateDestroyCommand(netID);
	auto len = message.size();
	assert(len < numeric_limits<uint32_t>::max());	// message is too long!
    for (auto connection : clients) {
        net_interface->SendMessageToConnection(connection, message.c_str(), static_cast<uint32_t>(len), k_nSteamNetworkingSend_Reliable, nullptr);
    }
}

void RavEngine::NetworkServer::SendMessageToAllClients(const std::string_view& msg, Reliability mode) const
{
	for (const auto connection : clients) {
		SendMessageToClient(msg, connection, mode);
	}
}

void NetworkServer::SendMessageToClient(const std::string_view& msg, HSteamNetConnection connection, Reliability mode) const{
	assert(msg.size() < numeric_limits<uint32_t>::max());	// message is too long!
	net_interface->SendMessageToConnection(connection, msg.data(), static_cast<uint32_t>(msg.length()), mode, nullptr);
}

void RavEngine::NetworkServer::SendMessageToAllClientsExcept(const std::string_view& msg, HSteamNetConnection connection, Reliability mode) const
{
	for (const auto c : clients) {
		if (c != connection) {
			SendMessageToClient(msg,c,mode);
		}
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
	listenSocket = net_interface->CreateListenSocketIP(serverLocalAddr, 1, &opt);
	
	if (listenSocket == k_HSteamListenSocket_Invalid )
		Debug::Fatal( "Failed to listen on port {}", port );
	
	pollGroup = net_interface->CreatePollGroup();
	if (pollGroup == k_HSteamNetPollGroup_Invalid)
		Debug::Fatal("Failed to create poll group");
	
	Debug::Log("Listening on port {}",port);
	
	currentServer = this;
	
	workerIsRunning = true;
	workerHasStopped = false;
	worker = std::thread(&NetworkServer::ServerTick,this);
	worker.detach();
}

void NetworkServer::Stop(){
	// kick all the clients
	for (const auto& con : clients) {
		net_interface->CloseConnection(con,0,nullptr,false);
	}

	workerIsRunning = false;	//this unblocks the worker thread, allowing it to exit
	while(!workerHasStopped);	//wait for thread to exit
	net_interface->CloseListenSocket(listenSocket);
	net_interface->DestroyPollGroup(pollGroup);
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
			if ( net_interface->AcceptConnection( pInfo->m_hConn ) != k_EResultOK )
			{
				// This could fail.  If the remote host tried to connect, but then
				// disconnected, the connection may already be half closed.  Just
				// destroy whatever we have on our side.
				net_interface->CloseConnection( pInfo->m_hConn, 0, nullptr, false );
				Debug::Error("Can't accept connection, was it already closed?");
				break;
			}
			
			// Assign the poll group
			if ( !net_interface->SetConnectionPollGroup( pInfo->m_hConn, pollGroup ) )
			{
				net_interface->CloseConnection( pInfo->m_hConn, 0, nullptr, false );
				Debug::Error( "Failed to set poll group" );
				break;
			}
			
			//track the connection
			clients.insert(pInfo->m_hConn);
			if(OnClientConnecting){
				OnClientConnecting(pInfo->m_hConn);
			}
			break;
		case k_ESteamNetworkingConnectionState_FindingRoute:
			
			break;
		case k_ESteamNetworkingConnectionState_Connected:
			// We will get a callback immediately after accepting the connection.
			// Since we are the server, we can ignore this, it's not news to us.
			if(OnClientConnected){
				OnClientConnected(pInfo->m_hConn);
			}
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
				
			}
			else{
				assert( pInfo->m_eOldState == k_ESteamNetworkingConnectionState_Connecting );
			}
			
			// Clean up the connection.  This is important!
			// The connection is "closed" in the network sense, but
			// it has not been destroyed.  We must close it on our end, too
			// to finish up.  The reason information do not matter in this case,
			// and we cannot linger because it's already closed on the other end,
			DisconnectClient(pInfo->m_hConn,0,"Automatic disconnection");

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
			int numMsgs = net_interface->ReceiveMessagesOnPollGroup( pollGroup, &pIncomingMsg, 1 );
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
				OnRPC(message, pIncomingMsg->GetConnection());
                break;
			case NetworkBase::CommandCode::ClientRequestingWorldSynchronization:
				SynchronizeWorldToClient(pIncomingMsg->GetConnection(), message);
				break;
            default:
                Debug::Warning("Invalid command code: {}",cmdcode);
            }
			
			//deallocate when done
			pIncomingMsg->Release();
		}
				
		//invoke callbacks
		net_interface->RunCallbacks();
	}
	workerHasStopped = true;
}

void RavEngine::NetworkServer::SynchronizeWorldToClient(HSteamNetConnection connection, const std::string_view& in_message)
{
    char buffer[World::id_size]{0};
	std::memcpy(buffer, in_message.data() + 1, in_message.size() - 1);
	string name(buffer,sizeof(buffer));
	if (auto world = GetApp()->GetWorldByName(name)) {
		// get all the networkidentities in the world
		auto identities = world.value()->GetAllComponentsOfType<NetworkIdentity>();
		// call SpawnEntity on each owner
        
		for (const auto& identity : *identities) {
			auto entity = identity.GetOwner();
			auto id = identity.GetNetTypeID();
			auto& netID = identity.GetNetworkID();
			//send highest-priority safe message with this info to clients
			auto message = CreateSpawnCommand(netID, id, world.value()->worldID);

			SendMessageToClient(message, connection,Reliability::Reliable);
		}
	}
}

void RavEngine::NetworkServer::OnRPC(const std::string_view& cmd, HSteamNetConnection origin)
{
	//decode the RPC header to to know where it is going

	uuids::uuid id(cmd.data() + 1);
	if (!NetworkIdentities.if_contains(id, [&cmd, &origin](auto entity) {
		assert(entity.template HasComponent<NetworkIdentity>());
		bool isOwner = origin == entity.template GetComponent<NetworkIdentity>().Owner;
		entity.template GetComponent<RPCComponent>().CacheServerRPC(cmd, isOwner, origin);
		})) {
		
            Debug::Warning("Got RPC for {} but it has not been tracked, ids = ",id.to_string());
            for (const auto& [id,entity] : NetworkIdentities) {
                Debug::Warning(" - UUID = {}, id = {}", id.to_string(), entity_id_t(entity.id.id));
            }
	}
}

void RavEngine::NetworkServer::DisconnectClient(HSteamNetConnection con, int reason, const char* msg_optional)
{
	// so we just pass 0's.
	net_interface->CloseConnection(con, reason, msg_optional, false);

	// need to destroy any entities owned by this client
    GetApp()->DispatchMainThread([this,con]{
        HandleDisconnect(con);

        if (OnClientDisconnected) {
            OnClientDisconnected(con);
        }
    });
}

void RavEngine::NetworkServer::ChangeOwnership(HSteamNetConnection newOwner, ComponentHandle<NetworkIdentity> object)
{
	//send message revoke ownership for the existing owner, if it is not currently owned by server
	if (object->Owner != k_HSteamNetConnection_Invalid) {
		auto& uuid = object->GetNetworkID();
		char msg[uuids::uuid::size() + 1];
		msg[0] = NetworkBase::CommandCode::OwnershipRevoked;
		std::memcpy(msg + 1, uuid.raw(), uuid.size());
        OwnershipTracker[object->Owner].erase(object);
		SendMessageToClient(std::string_view(msg, sizeof(msg)), object->Owner, Reliability::Reliable);
	}

	//update the object's ownership value
	object->Owner = newOwner;

	//send message to the new owner that it is now the owner, if the new owner is not the server
	if (newOwner != k_HSteamNetConnection_Invalid) {
		const auto& uuid = object->GetNetworkID();
        char msg[uuids::uuid::size() + 1]{0};
		msg[0] = NetworkBase::CommandCode::OwnershipToThis;
		std::memcpy(msg + 1, uuid.raw(), uuid.size());
        OwnershipTracker[object->Owner].insert(object);
		SendMessageToClient(std::string_view(msg, sizeof(msg)), object->Owner, Reliability::Reliable);
	}
}

std::string RavEngine::NetworkServer::CreateSpawnCommand(const uuids::uuid& id, ctti_t type, std::string_view& worldID)
{
    constexpr uint16_t size = 16 + sizeof(type) + World::id_size + 1;
    char message[size];
    memset(message, 0, size);

    //set command code
    message[0] = CommandCode::Spawn;

    char offset = 1;
    //set type
    memcpy(message + offset, &type, sizeof(type));

    offset += sizeof(type);

    //set uuid
    auto raw = id.raw();
    memcpy(message + offset, raw, 16);
    Debug::Log("Server spawned {} - {}", type, id.to_string());
    offset += 16;

    //set worldid
    memcpy(message + offset, worldID.data(), World::id_size);

    return string(message,size);
}

std::string RavEngine::NetworkServer::CreateDestroyCommand(const uuids::uuid& id)
{
    constexpr uint16_t size = 16 + 1;
    char message[size];
    
    //set command code
    message[0] = CommandCode::Destroy;
    
    //set uuid
    auto raw = id.raw();
    memcpy(message + 1, raw, 16);
    
    return string(message,size);
}

