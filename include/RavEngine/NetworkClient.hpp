#pragma once
#include <string>
#include <cstdint>
#include "NetworkBase.hpp"
#include <steam/isteamnetworkingsockets.h>
#include "ComponentHandle.hpp"
#include "Function.hpp"
#include "Ref.hpp"

namespace RavEngine {
	class World;
	struct Entity;
	class World;
	struct NetworkIdentity;

class NetworkClient : public NetworkBase{
    UnorderedMap<ctti_t, Function<void(Entity, Ref<World>)>> OnNetSpawnHooks;
	void RevokeOwnership(ComponentHandle<NetworkIdentity> id);
	void GainOwnership(ComponentHandle<NetworkIdentity> id);
public:
	NetworkClient();
	void Connect(const std::string& addr, uint16_t port);
	void Disconnect();
	~NetworkClient();	//gracefully disconnect
	static void SteamNetConnectionStatusChanged(SteamNetConnectionStatusChangedCallback_t*);

	void SendMessageToServer(const std::string_view& msg, Reliability mode) const;

	void OnRPC(const std::string_view& cmd);

	void SendSyncWorldRequest(Ref<World> world);

	template<typename T>
    constexpr inline void SetNetSpawnHook(const decltype(OnNetSpawnHooks)::value_type::second_type& func) {
		OnNetSpawnHooks.insert(std::make_pair(CTTI<T>(),func));
	}
	
	//attach event listeners here
	Function<void(HSteamNetConnection)> OnConnecting, OnConnected, OnLostConnection;
	
protected:
	ISteamNetworkingSockets *net_interface = nullptr;
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

	static NetworkClient* currentClient;
};

}
