#include "NetworkClient.hpp"
#include "Debug.hpp"
#include <steam/isteamnetworkingutils.h>	//this is required in for ParseString
#include "App.hpp"
#include "RPCComponent.hpp"
#include "SyncVar.hpp"

using namespace RavEngine;
NetworkClient* NetworkClient::currentClient = nullptr;

NetworkClient::NetworkClient(){
	interface = SteamNetworkingSockets();
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
	connection = interface->ConnectByIPAddress(ip, 1, &options);
	if (connection == k_HSteamNetConnection_Invalid){
		Debug::Fatal("Cannot connect to {}:{}",address,port);
	}
	currentClient = this;
	workerIsRunning = true;
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
			interface->CloseConnection( pInfo->m_hConn, 0, nullptr, false );
			connection = k_HSteamNetConnection_Invalid;
			break;
		}
			
		case k_ESteamNetworkingConnectionState_Connecting:
			// We will get this callback when we start connecting.
			// We can ignore this.
			break;
			
		case k_ESteamNetworkingConnectionState_Connected:
			Debug::Log("Connected to remote host");
			break;
			
		default:
			// Silences -Wswitch
			break;
	}
}

void NetworkClient::Disconnect(){
	workerIsRunning = false;
	while (!workerHasStopped);
	interface->CloseConnection(connection, 0, nullptr, false);

}

NetworkClient::~NetworkClient(){
	Disconnect();
}

//routing call
void NetworkClient::SteamNetConnectionStatusChanged(SteamNetConnectionStatusChangedCallback_t * pInfo){
	currentClient->OnSteamNetConnectionStatusChanged(pInfo);
}

void NetworkClient::ClientTick(){
	while(workerIsRunning){
		while(workerIsRunning){
			ISteamNetworkingMessage *pIncomingMsg = nullptr;
			int numMsgs = interface->ReceiveMessagesOnConnection( connection, &pIncomingMsg, 1 );
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
			case NetworkBase::CommandCode::SyncVar:
				//TODO: check ownership
				SyncVar_base::EnqueueCmd(message);
				break;
            default:
                Debug::Warning("Invalid command code: {}",cmdcode);
            }
			
			// We don't need this anymore.
			pIncomingMsg->Release();
		}
		
		//state changes
		interface->RunCallbacks();
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
        
    //find the world and spawn
    if (auto e = App::networkManager.CreateEntity(id, uuid)){
        if (auto world = App::GetWorldByName(std::string(worldname,World::id_size))){
            world.value()->Spawn(e.value());
            auto netid = e.value()->GetComponent<NetworkIdentity>().value();
            if (netid){
                Debug::Assert(netid->GetNetworkID() == uuid, "Created object does not have correct NetID! {} != {}",uuid.to_string(), netid->GetNetworkID().to_string());
                
                NetworkIdentities[netid->GetNetworkID()] = netid;
            }
            else{
                Debug::Fatal("Cannot spawn networked entity without NetworkIdentity! Check uuid constructor.");
            }
        }
        else{
            Debug::Fatal("Cannot spawn networked entity in unloaded world: {}", worldname);
        }
    }
    else{
        Debug::Fatal("Cannot spawn entity with type ID {}",id);
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
	if (NetworkIdentities.if_contains(uuid, [&](const Ref<NetworkIdentity>& id) {
		auto owner = id->getOwner().lock();
		if (owner) {
			owner->Destroy();
		}
		NetworkIdentities.erase(uuid);
	})) {}
    else{
        Debug::Warning("Cannot destroy entity with UUID {} because it does not exist",uuid.to_string());
    }
}

void RavEngine::NetworkClient::OwnershipRevoked(const std::string_view& cmd)
{
	uuids::uuid id(cmd.data() + 1);
	NetworkIdentities.if_contains(id, [this](const Ref<NetworkIdentity>& id) {
		id->Owner = k_HSteamListenSocket_Invalid;
	});
}

void RavEngine::NetworkClient::OwnershipToThis(const std::string_view& cmd)
{
	uuids::uuid id(cmd.data() + 1);
	NetworkIdentities.if_contains(id, [this](const Ref<NetworkIdentity>& id) {
		id->Owner = 30;	//any number = this machine has ownership
	});
}

void NetworkClient::SendMessageToServer(const std::string_view& msg) const {
	interface->SendMessageToConnection(connection, msg.data(), msg.length(), k_nSteamNetworkingSend_Reliable, nullptr);
}

void RavEngine::NetworkClient::OnRPC(const std::string_view& cmd)
{
	//decode the RPC header to to know where it is going
	uuids::uuid id(cmd.data() + 1);
	NetworkIdentities.if_contains(id, [&](const Ref<NetworkIdentity>& netid) {
		auto entity = netid->getOwner().lock();
		entity->GetComponent<RPCComponent>().value()->CacheClientRPC(cmd, netid->Owner == k_HSteamNetConnection_Invalid, connection);
	});
}
