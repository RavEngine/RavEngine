#pragma once
#include <cstdint>
#include "NetworkBase.hpp"
#include <steam/isteamnetworkingsockets.h>
#include "DataStructures.hpp"
#include "Ref.hpp"
#include <uuids.h>
#include <phmap.h>

namespace RavEngine {
	class Entity;

class NetworkServer : public NetworkBase{
public:
	NetworkServer();
	void Start(uint16_t port);
	void Stop();
	~NetworkServer();	//calls stop
	static void SteamNetConnectionStatusChanged(SteamNetConnectionStatusChangedCallback_t*);

	void SpawnEntity(Ref<Entity> e);
	void DestroyEntity(Ref<Entity> e);
protected:
	ISteamNetworkingSockets *interface = nullptr;
	HSteamListenSocket listenSocket = k_HSteamListenSocket_Invalid;
	HSteamNetPollGroup pollGroup = k_HSteamNetPollGroup_Invalid;
	void OnSteamNetConnectionStatusChanged(SteamNetConnectionStatusChangedCallback_t*);
	
	phmap::flat_hash_set<HSteamNetConnection> clients;
	
	static NetworkServer* currentServer;
	
	void ServerTick();
    
    //stores the ownership, see who owns a particular object
    locked_node_hashmap<uuids::uuid, HSteamNetConnection> ownership;
	
};

}
