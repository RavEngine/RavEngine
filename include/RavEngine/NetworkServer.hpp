#pragma once
#include <cstdint>
#include <steam/isteamnetworkingsockets.h>
#include "DataStructures.hpp"
#include <uuids.h>

namespace RavEngine {

class NetworkServer{
public:
	NetworkServer();
	void Start(uint16_t port);
	void Stop();
	~NetworkServer();	//calls stop
	static void SteamNetConnectionStatusChanged(SteamNetConnectionStatusChangedCallback_t*);
protected:
	ISteamNetworkingSockets *interface;
	HSteamListenSocket listenSocket;
	HSteamNetPollGroup pollGroup;
	void OnSteamNetConnectionStatusChanged(SteamNetConnectionStatusChangedCallback_t*);
    
    //stores the ownership, see who owns a particular object
    locked_node_hashmap<uuids::uuid, HSteamNetConnection> ownership;
};

}
