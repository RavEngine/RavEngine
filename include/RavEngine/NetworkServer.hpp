#pragma once
#include <rpc/rpc.h>
#include <rpc/server.h>

namespace RavEngine {

class NetworkServer{
public:
	NetworkServer(uint16_t port);
	void Start();
	void Stop();
	~NetworkServer();	//calls stop
protected:
	rpc::server server;
};

}
