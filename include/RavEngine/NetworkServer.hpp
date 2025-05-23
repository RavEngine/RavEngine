#pragma once
#include <cstdint>
#include "NetworkBase.hpp"
#include <steam/isteamnetworkingsockets.h>
#include "DataStructures.hpp"
#include "Ref.hpp"
#include <phmap.h>
#include "Function.hpp"
#include "ComponentHandle.hpp"
#include <string_view>

namespace RavEngine {
	struct Entity;
	struct NetworkIdentity;

class NetworkServer : public NetworkBase{
public:
	NetworkServer();
	void Start(uint16_t port);
	void Stop();
	~NetworkServer();	//calls stop
	static void SteamNetConnectionStatusChanged(SteamNetConnectionStatusChangedCallback_t*);

	void SpawnEntity(World*, ctti_t, Entity, const uuids::uuid&);
	void DestroyEntity(const uuids::uuid&);

	void SendMessageToAllClients(const std::string_view& msg, Reliability mode) const;

	void SendMessageToClient(const std::string_view& msg, HSteamNetConnection connection, Reliability mode) const;

	void SendMessageToAllClientsExcept(const std::string_view& msg, HSteamNetConnection connection, Reliability mode) const;

	/**
	* Disconnect a client from the server
	* @param the connection handle to disconnect
	* @param reason an integer specifying the reason. This is user-defined.
	* @param msg_optional an optional string message to provide which will be received on the client.
	*/
	void DisconnectClient(HSteamNetConnection con, int reason, const char* msg_optional = nullptr);

	/**
	Change the ownership of a networked object
	@param newOwner the connection handle for the new owner. Use Invalid to make the server owner
	@param object the networkidentity to udpate the ownership of
	*/
	void ChangeOwnership(HSteamNetConnection newOwner, ComponentHandle<NetworkIdentity> object);
		
	//attach event listeners here
	Function<void(HSteamNetConnection)> OnClientConnecting, OnClientConnected, OnClientDisconnected;
    
    std::string CreateSpawnCommand(const uuids::uuid& id, ctti_t type, std::string_view& worldID);

    std::string CreateDestroyCommand(const uuids::uuid& id);
	
protected:
	void OnRPC(const std::string_view& cmd, HSteamNetConnection);

	ISteamNetworkingSockets *net_interface = nullptr;
	HSteamListenSocket listenSocket = k_HSteamListenSocket_Invalid;
	HSteamNetPollGroup pollGroup = k_HSteamNetPollGroup_Invalid;
	void OnSteamNetConnectionStatusChanged(SteamNetConnectionStatusChangedCallback_t*);
	
    locked_node_hashset<HSteamNetConnection,SpinLock> clients;

	// since the server owns all objects by default, we only need to update this
	// when the ownership changes to a non-server client. If an object is not in this
	// structure, we assume the server owns it.
    UnorderedMap<HSteamNetConnection, UnorderedSet<ComponentHandle<NetworkIdentity>>> OwnershipTracker;

	// invoked automatically when a client disconnects. We destroy the entities it owns.
	void HandleDisconnect(HSteamNetConnection);
	
	static NetworkServer* currentServer;
	
	void ServerTick();

	//invoked when clients request to have their worlds synchronized
	void SynchronizeWorldToClient(HSteamNetConnection connection, const std::string_view& in_message);

public:
	/**
	* Get the internal client structure. Note that to track application-specific client data, create your own datastructure and add event listeners to the OnConnecting, Ondisconnected, etc functions and track it yourself.
	*/
    constexpr inline const decltype(clients)& GetClients() {
		return clients;
	}
};

}
