#pragma once
#include <string>
#include <cstdint>
#include <steam/isteamnetworkingsockets.h>

namespace RavEngine {

class NetworkClient{
public:
	NetworkClient();
	void Connect(const std::string& addr, uint16_t port);
	void Disconnect();
	~NetworkClient();	//gracefully disconnect
	static void SteamNetConnectionStatusChanged(SteamNetConnectionStatusChangedCallback_t*) ;
protected:
	ISteamNetworkingSockets *interface;
	HSteamNetConnection connection;
	void OnSteamNetConnectionStatusChanged(SteamNetConnectionStatusChangedCallback_t*) ;
};

}
