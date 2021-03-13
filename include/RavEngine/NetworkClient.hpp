#pragma once
#include <string>
#include <cstdint>
#include <steam/isteamnetworkingsockets.h>
#include <thread>

namespace RavEngine {

class NetworkClient{
public:
	NetworkClient();
	void Connect(const std::string& addr, uint16_t port);
	void Disconnect();
	~NetworkClient();	//gracefully disconnect
	static void SteamNetConnectionStatusChanged(SteamNetConnectionStatusChangedCallback_t*);
protected:
	ISteamNetworkingSockets *interface = nullptr;
	HSteamNetConnection connection = k_HSteamNetConnection_Invalid;
	std::thread workerThread;
	std::atomic<bool> clientIsRunning = false;
	std::atomic<bool> clientHasStopped = false;
	void OnSteamNetConnectionStatusChanged(SteamNetConnectionStatusChangedCallback_t*);
	
	void ClientTick();
	
	static NetworkClient* currentClient;
};

}
