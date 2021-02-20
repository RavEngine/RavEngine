#pragma once
#include "NetworkServer.hpp"
#include "NetworkClient.hpp"

namespace RavEngine {

class NetworkManager{
public:
	static bool IsServer();
	
	static bool IsClient();
	
	std::unique_ptr<NetworkServer> server;
	std::unique_ptr<NetworkClient> client;
};

}
