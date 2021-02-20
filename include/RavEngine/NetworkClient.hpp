#pragma once
#include <string>
#include <cstdint>
#include <rpc/client.h>

namespace RavEngine {

class NetworkClient{
public:
	NetworkClient(const std::string& address, const uint16_t port);
	void Disconnect();
	~NetworkClient();	//gracefully disconnect
protected:
	rpc::client client;
};

}
