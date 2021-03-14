#include "NetworkClient.hpp"
#include "Debug.hpp"
#include <steam/isteamnetworkingutils.h>	//this is required in for ParseString
#include "App.hpp"

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
			App::networkManager.OnMessageReceived(message);
			
			// We don't need this anymore.
			pIncomingMsg->Release();
		}
		
		//state changes
		interface->RunCallbacks();
	}
	workerHasStopped = true;
}
