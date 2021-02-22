#include "NetworkClient.hpp"

using namespace RavEngine;

NetworkClient::NetworkClient(){
	interface = SteamNetworkingSockets();
}

void NetworkClient::Connect(const std::string& address, uint16_t port){
	SteamNetworkingIdentity addr;		//TODO: convert address to networking identity
	SteamNetworkingConfigValue_t options;
	connection = interface->ConnectP2P(addr, port, 1, &options);
}

void NetworkClient::Disconnect(){
	interface->CloseConnection(connection, 0, nullptr, false);
}

NetworkClient::~NetworkClient(){
	Disconnect();
}
