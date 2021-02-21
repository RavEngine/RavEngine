#pragma once
#include <cstdint>
#include <steam/isteamnetworkingsockets.h>

namespace RavEngine {

class NetworkServer{
public:
	NetworkServer(uint16_t port);
	void Start();
	void Stop();
	~NetworkServer();	//calls stop
protected:
};

}
