#pragma once
#include <string>
#include <cstdint>
#include <steam/isteamnetworkingsockets.h>

namespace RavEngine {

class NetworkClient{
public:
	NetworkClient(const std::string& address, const uint16_t port);
	void Disconnect();
	~NetworkClient();	//gracefully disconnect
protected:
};

}
