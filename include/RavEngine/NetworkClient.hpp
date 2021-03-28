#pragma once
#include <string>
#include <cstdint>
#include "NetworkBase.hpp"
#include <steam/isteamnetworkingsockets.h>
#include <thread>

namespace RavEngine {

class NetworkClient : public NetworkBase{
public:
	NetworkClient();
	void Connect(const std::string& addr, uint16_t port);
	void Disconnect();
	~NetworkClient();	//gracefully disconnect
	static void SteamNetConnectionStatusChanged(SteamNetConnectionStatusChangedCallback_t*);

	void SendMessageToServer(const std::string_view& msg, Reliability mode) const;

	void OnRPC(const std::string_view& cmd);
protected:
	ISteamNetworkingSockets *interface = nullptr;
	HSteamNetConnection connection = k_HSteamNetConnection_Invalid;
	void OnSteamNetConnectionStatusChanged(SteamNetConnectionStatusChangedCallback_t*);
	
	void ClientTick();
    
    /**
     Invoked when spawn command is received
     @param cmd the raw command from server
     */
    void NetSpawn(const std::string_view& cmd);
    
    /**
     Invoked when destroy command is received
     @param cmd the raw command from server
     */
    void NetDestroy(const std::string_view& cmd);
	
	void OwnershipRevoked(const std::string_view& cmd);
	void OwnershipToThis(const std::string_view& cmd);
	
	void SyncVarOwnershipRevoked(const std::string_view& cmd);
	void SyncVarOwnershipToThis(const std::string_view& cmd);

	static NetworkClient* currentClient;
};

}
