#pragma once
#include "Component.hpp"
#include "Queryable.hpp"
#include <uuids.h>
#include <steam/isteamnetworkingutils.h>

namespace RavEngine {
	struct NetworkIdentity : public Component, public Queryable< NetworkIdentity>{
	private:
        const uuids::uuid NetworkID;
	public:
		
		const bool triggerMessage;
		
		//default constructor - used on Server
		NetworkIdentity() : NetworkID(uuids::uuid::create()), triggerMessage(true){}
		
		//Used on clients
		NetworkIdentity(const uuids::uuid& id) : NetworkID(id), triggerMessage(false){}
		
		inline decltype(NetworkID) GetNetworkID() const{
			return NetworkID;
		}
		
		HSteamNetConnection Owner = k_HSteamNetConnection_Invalid;	//invalid = server
	};
}
