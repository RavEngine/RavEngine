#pragma once
#include <cstdint>
#include <steam/isteamnetworkingsockets.h>
#include "DataStructures.hpp"
#include <uuids.h>
#include <phmap.h>

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
	
	std::optional<std::thread> worker;
	std::atomic<bool> serverIsRunning = false;
	phmap::flat_hash_set<HSteamNetConnection> clients;
	
	void ServerTick();
    
    //stores the ownership, see who owns a particular object
    locked_node_hashmap<uuids::uuid, HSteamNetConnection> ownership;
	
};

}
