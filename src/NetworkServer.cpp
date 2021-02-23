#include "NetworkServer.hpp"
#include "Debug.hpp"

using namespace RavEngine;

NetworkServer::NetworkServer(){
	interface = SteamNetworkingSockets();
}

void NetworkServer::Start(uint16_t port){
	//configure and start server
	SteamNetworkingConfigValue_t opt;
	opt.SetPtr(k_ESteamNetworkingConfig_Callback_ConnectionStatusChanged, [&](SteamNetConnectionStatusChangedCallback_t *pInfo){
		//TODO: replace with member function later
		OnSteamNetConnectionStatusChanged(pInfo);
		
	});
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
}

void NetworkServer::Stop(){
	interface->CloseListenSocket(listenSocket);
	interface->DestroyPollGroup(pollGroup);
}

NetworkServer::~NetworkServer(){
	Stop();
}

void NetworkServer::OnSteamNetConnectionStatusChanged(SteamNetConnectionStatusChangedCallback_t *pInfo){
	
}
