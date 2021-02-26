#include "NetworkClient.hpp"
#include "Debug.hpp"

using namespace RavEngine;

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
}

void NetworkClient::OnSteamNetConnectionStatusChanged(SteamNetConnectionStatusChangedCallback_t * pInfo){
	Debug::Log("Status changed!");
}

void NetworkClient::Disconnect(){
	interface->CloseConnection(connection, 0, nullptr, false);
}

NetworkClient::~NetworkClient(){
	Disconnect();
}

//routing call
void NetworkClient::SteamNetConnectionStatusChanged(SteamNetConnectionStatusChangedCallback_t * pInfo){
	static_cast<NetworkClient*>((void*)pInfo->m_info.m_nUserData)->OnSteamNetConnectionStatusChanged(pInfo);
}
