#include "NetworkClient.hpp"

using namespace RavEngine;

NetworkClient::NetworkClient(const std::string& addr, const uint16_t port) : client(addr, port){
	
}

void NetworkClient::Disconnect(){
	//TODO: disconnect from server
}

NetworkClient::~NetworkClient(){
	Disconnect();
}
