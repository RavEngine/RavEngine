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
		
		HSteamNetConnection Owner = k_HSteamNetConnection_Invalid;	

		/** On the server:
		*	invalid = the server has ownership
		*	<any number> = the machine on the specified connection has ownership
		*
		* On the client:
		*	invalid = this machine does __not__ have ownership
		*	<any number> = this machine has ownership
		*/
		bool IsOwner() const;
	};
}
