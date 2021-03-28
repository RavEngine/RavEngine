#pragma once
#include <cstdint>
#include "NetworkBase.hpp"
#include <steam/isteamnetworkingsockets.h>
#include "DataStructures.hpp"
#include "Ref.hpp"
#include <uuids.h>
#include <phmap.h>
#include <functional>

namespace RavEngine {
	class Entity;
	class SyncVar_base;

class NetworkServer : public NetworkBase{
public:
	NetworkServer();
	void Start(uint16_t port);
	void Stop();
	~NetworkServer();	//calls stop
	static void SteamNetConnectionStatusChanged(SteamNetConnectionStatusChangedCallback_t*);

	void SpawnEntity(Ref<Entity> e);
	void DestroyEntity(Ref<Entity> e);

	void SendMessageToAllClients(const std::string_view& msg, Reliability mode) const;

	void SendMessageToClient(const std::string_view& msg, HSteamNetConnection connection, Reliability mode) const;

	void OnRPC(const std::string_view& cmd, HSteamNetConnection);

	/**
	Change the ownership of a networked object
	@param newOwner the connection handle for the new owner. Use Invalid to make the server owner
	@param object the networkidentity to udpate the ownership of
	*/
	void ChangeOwnership(HSteamNetConnection newOwner, Ref<NetworkIdentity> object);
	
	void ChangeSyncVarOwnership(HSteamNetConnection newOwner, SyncVar_base& var);
	
	//attach event listeners here
	std::function<void(HSteamNetConnection)> OnClientConnecting, OnClientConnected, OnClientDisconnected;
	
protected:
	ISteamNetworkingSockets *interface = nullptr;
	HSteamListenSocket listenSocket = k_HSteamListenSocket_Invalid;
	HSteamNetPollGroup pollGroup = k_HSteamNetPollGroup_Invalid;
	void OnSteamNetConnectionStatusChanged(SteamNetConnectionStatusChangedCallback_t*);
	
	phmap::flat_hash_set<HSteamNetConnection> clients;
	
	static NetworkServer* currentServer;
	
	void ServerTick();
};

}
