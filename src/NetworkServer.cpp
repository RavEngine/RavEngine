#include "NetworkServer.hpp"
#include "Debug.hpp"

using namespace RavEngine;

NetworkServer::NetworkServer(){
	interface = SteamNetworkingSockets();
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
	
	serverIsRunning = true;
	worker.emplace(&NetworkServer::ServerTick,this);
}

void NetworkServer::Stop(){
	serverIsRunning = false;	//this unblocks the worker thread, allowing it to exit
	interface->CloseListenSocket(listenSocket);
	interface->DestroyPollGroup(pollGroup);
}

NetworkServer::~NetworkServer(){
	Stop();
}

void NetworkServer::OnSteamNetConnectionStatusChanged(SteamNetConnectionStatusChangedCallback_t *pInfo){
	
}

void NetworkServer::SteamNetConnectionStatusChanged(SteamNetConnectionStatusChangedCallback_t * pInfo){
	static_cast<NetworkServer*>((void*)pInfo->m_info.m_nUserData)->OnSteamNetConnectionStatusChanged(pInfo);
}

void NetworkServer::ServerTick(){
	while(serverIsRunning){
		
		//get incoming messages
		while (serverIsRunning){	//do we need this double loop?
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
			auto data = pIncomingMsg->m_pData;
			auto nbytes = pIncomingMsg->m_cbSize;
		}
				
		//invoke callbacks
		interface->RunCallbacks();
	}
}
