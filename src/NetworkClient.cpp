#include "NetworkClient.hpp"
#include "Debug.hpp"
#include <steam/isteamnetworkingutils.h>	//this is required in for ParseString
#include "App.hpp"
#include "RPCComponent.hpp"
#include "World.hpp"
#include <string_view>
#include <cstring>
#include <limits>
#include "NetworkIdentity.hpp"

using namespace RavEngine;
NetworkClient* NetworkClient::currentClient = nullptr;

void RavEngine::NetworkClient::RevokeOwnership(ComponentHandle<NetworkIdentity> id) {
	id->Owner = k_HSteamListenSocket_Invalid;
}

void RavEngine::NetworkClient::GainOwnership(ComponentHandle<NetworkIdentity> id) {
	id->Owner = 30;	//any number = this machine has ownership
}

NetworkClient::NetworkClient(){
	net_interface = SteamNetworkingSockets();
}

void NetworkClient::Connect(const std::string& address, uint16_t port){
	SteamNetworkingIPAddr ip;
	ip.Clear();
	if(!ip.ParseString(address.c_str())){
		Debug::Fatal("Invalid IP: {}",address);
	}
	ip.m_port = port;
	
	SteamNetworkingConfigValue_t options;
	options.SetPtr(k_ESteamNetworkingConfig_ConnectionUserData, (void*)this);	//the thisptr
	options.SetPtr(k_ESteamNetworkingConfig_Callback_ConnectionStatusChanged,  (void*)NetworkClient::SteamNetConnectionStatusChanged);
	connection = net_interface->ConnectByIPAddress(ip, 1, &options);
	if (connection == k_HSteamNetConnection_Invalid){
		Debug::Fatal("Cannot connect to {}:{}",address,port);
	}
	currentClient = this;
	workerIsRunning = true;
	workerHasStopped = false;
	worker = std::thread(&NetworkClient::ClientTick, this);
	worker.detach();
}

void NetworkClient::OnSteamNetConnectionStatusChanged(SteamNetConnectionStatusChangedCallback_t * pInfo){
	switch ( pInfo->m_info.m_eState )
	{
		case k_ESteamNetworkingConnectionState_None:
			// NOTE: We will get callbacks here when we destroy connections.  You can ignore these.
			break;
			
		case k_ESteamNetworkingConnectionState_ClosedByPeer:
		case k_ESteamNetworkingConnectionState_ProblemDetectedLocally:
		{
			workerIsRunning = false;
			
			// Print an appropriate message
			if ( pInfo->m_eOldState == k_ESteamNetworkingConnectionState_Connecting )
			{
				// Note: we could distinguish between a timeout, a rejected connection,
				// or some other transport problem.
				Debug::Error("Cannot connect to remote host: {}",pInfo->m_info.m_szEndDebug);
			}
			else if ( pInfo->m_info.m_eState == k_ESteamNetworkingConnectionState_ProblemDetectedLocally )
			{
				Debug::Error("Connection dropped to remote host: {}",pInfo->m_info.m_szEndDebug);
			}
			else
			{
				// NOTE: We could check the reason code for a normal disconnection
				Debug::Log("Connection closed by remote host: {}",pInfo->m_info.m_szEndDebug);
			}
			
			// Clean up the connection.  This is important!
			// The connection is "closed" in the network sense, but
			// it has not been destroyed.  We must close it on our end, too
			// to finish up.  The reason information do not matter in this case,
			// and we cannot linger because it's already closed on the other end,
			// so we just pass 0's.
			net_interface->CloseConnection( pInfo->m_hConn, 0, nullptr, false );
			connection = k_HSteamNetConnection_Invalid;
			if(OnLostConnection){
				OnLostConnection(pInfo->m_hConn);
			}
			break;
		}
			
		case k_ESteamNetworkingConnectionState_Connecting:
			// We will get this callback when we start connecting.
			// We can ignore this.
			if (OnConnecting){
				OnConnecting(pInfo->m_hConn);
			}
			break;
			
		case k_ESteamNetworkingConnectionState_Connected:
			Debug::Log("Connected to remote host");
			if(OnConnected){
				OnConnected(pInfo->m_hConn);
			}
			break;
			
		default:
			// Silences -Wswitch
			break;
	}
}

void NetworkClient::Disconnect(){
	workerIsRunning = false;
	while (!workerHasStopped);
	net_interface->CloseConnection(connection, 0, nullptr, false);

}

NetworkClient::~NetworkClient(){
	Disconnect();
}

//routing call
void NetworkClient::SteamNetConnectionStatusChanged(SteamNetConnectionStatusChangedCallback_t * pInfo){
	if (currentClient){
		currentClient->OnSteamNetConnectionStatusChanged(pInfo);
	}
}

void NetworkClient::ClientTick(){
	while(workerIsRunning){
		while(workerIsRunning){
			ISteamNetworkingMessage *pIncomingMsg = nullptr;
			int numMsgs = net_interface->ReceiveMessagesOnConnection( connection, &pIncomingMsg, 1 );
			if ( numMsgs == 0 ){
				break;
			}
			if ( numMsgs < 0 ){
				Debug::Fatal( "Error checking for messages" );
			}
				
			std::string_view message((char*)pIncomingMsg->m_pData, pIncomingMsg->m_cbSize);
            //get the command code (first byte in the message)
            uint8_t cmdcode = message[0];
            switch (cmdcode) {
            case NetworkBase::CommandCode::Spawn:
                NetSpawn(message);
                break;
            case NetworkBase::CommandCode::Destroy:
                NetDestroy(message);
                break;
            case NetworkBase::CommandCode::RPC:
				OnRPC(message);
                break;
			case NetworkBase::CommandCode::OwnershipRevoked:
				OwnershipRevoked(message);
				break;
			case NetworkBase::CommandCode::OwnershipToThis:
				OwnershipToThis(message);
				break;
            default:
                Debug::Warning("Invalid command code: {}",cmdcode);
            }
			
			// We don't need this anymore.
			pIncomingMsg->Release();
		}
		
		//state changes
		net_interface->RunCallbacks();
	}
	workerHasStopped = true;
}

void NetworkClient::NetSpawn(const std::string_view& command){
    //unpack the command
    ctti_t id;
    uint8_t offset = 1;
    
    //CTTI id
    std::memcpy(&id,command.data()+offset,sizeof(id));
    offset += sizeof(id);
    
    // uuid
    char uuid_bytes[16];
    std::memcpy(uuid_bytes, command.data()+offset, 16);
    uuids::uuid uuid(uuid_bytes);
    offset += 16;
    
    // world name
    char worldname[World::id_size];
    std::memcpy(worldname, command.data()+offset, World::id_size);
    if (auto world = GetApp()->GetWorldByName(std::string(worldname,World::id_size))){
        // try to create the entity, the result of this is the entity spawned in the world
        GetApp()->DispatchMainThread([id,world,uuid,this](){
            if (auto e = GetApp()->networkManager.CreateEntity(id, world.value().get())){
                // create a NetworkIdentity for this spawned entity
                auto& eHandle = e.value();
                auto& netid = eHandle.EmplaceComponent<NetworkIdentity>(uuid);
                // track this entity
                NetworkIdentities[uuid] = eHandle;
                
                // now invoke the netspawnhook
                if (OnNetSpawnHooks.contains(id)) {
                    OnNetSpawnHooks.at(id)(e.value(),world.value());
                }
            }
            else{
                Debug::Fatal("Cannot spawn entity with CTTI ID {}",id);
            }
        });
    }
    else{
        Debug::Fatal("Cannot spawn networked entity in unloaded world: {}", worldname);
    }
}

void NetworkClient::NetDestroy(const std::string_view& command){
    //unpack the command
    uint8_t offset = 1;
    
    //uuid
    char uuid_bytes[16];
    std::memcpy(uuid_bytes,command.data() + offset,16);
    uuids::uuid uuid(uuid_bytes);
        
    //lookup the entity and destroy it
    GetApp()->DispatchMainThread([uuid,this](){
        if (NetworkIdentities.if_contains(uuid, [&](auto&& entity_pair) {
			entity_pair.second.Destroy();
        })) {
            //this must be here, otherwise we will encounter deadlock
            NetworkIdentities.erase(uuid);
        }
        else{
            Debug::Warning("Cannot destroy entity with UUID {} because it does not exist",uuid.to_string());
        }
    });
	
}

void RavEngine::NetworkClient::OwnershipRevoked(const std::string_view& cmd)
{
	uuids::uuid id(cmd.data() + 1);
	bool success = NetworkIdentities.if_contains(id, [this](auto&& id) {
		RevokeOwnership(id.second);
	});
	if (!success) {
		// the entity did not exist 
		Debug::Warning("Cannot revoke ownership from an entity that does not exist, id = {}", id.to_string());
	}
}

void RavEngine::NetworkClient::OwnershipToThis(const std::string_view& cmd)
{
	uuids::uuid netid(cmd.data() + 1);
    GetApp()->DispatchMainThread([netid,this](){
        bool success = NetworkIdentities.if_contains(netid, [this](auto&& id) {
            GainOwnership(id.second);
        });
        if (!success) {
            Debug::Warning("Cannot add ownership to an entity that does not exist, id = {}", netid.to_string());
        }
    });
}

void NetworkClient::SendMessageToServer(const std::string_view& msg, Reliability mode) const {
	assert(msg.length() < std::numeric_limits<uint32_t>::max());	// message is too long!
	net_interface->SendMessageToConnection(connection, msg.data(), static_cast<uint32_t>(msg.length()), mode, nullptr);
}

void RavEngine::NetworkClient::OnRPC(const std::string_view& cmd)
{
	//decode the RPC header to to know where it is going
	uuids::uuid id(cmd.data() + 1);
	bool success = NetworkIdentities.if_contains(id, [&cmd,this](auto&& entity_pair) {
		auto entity = entity_pair.second;
        assert(entity.template HasComponent<RPCComponent>());
        assert(entity.template HasComponent<NetworkIdentity>());
        auto& netid = entity.template GetComponent<NetworkIdentity>();
        entity.template GetComponent<RPCComponent>().CacheClientRPC(cmd, netid.Owner == k_HSteamNetConnection_Invalid, this->connection);
	});
	if (!success) {
		Debug::Warning("Cannot relay RPC, entity with ID {} does not exist", id.to_string());
	}
}

void RavEngine::NetworkClient::SendSyncWorldRequest(Ref<World> world) {
	// sending this command code + the world ID to spawn
	char buffer[1 + World::id_size]{ 0 };
	buffer[0] = CommandCode::ClientRequestingWorldSynchronization;
	std::memcpy(buffer + 1, world->worldID.data(), world->worldID.size());
	SendMessageToServer(std::string_view(buffer, sizeof(buffer)), Reliability::Reliable);
}
