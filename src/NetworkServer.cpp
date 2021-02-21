#include "NetworkServer.hpp"

using namespace RavEngine;

NetworkServer::NetworkServer(uint16_t port){}

void NetworkServer::Start(){
	//server.async_run(4);	//TODO: customize thread count
}

void NetworkServer::Stop(){
	//server.stop();
}

NetworkServer::~NetworkServer(){
	Stop();
}
