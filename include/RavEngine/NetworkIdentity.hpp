#pragma once
#include "ComponentWithOwner.hpp"
#include "Queryable.hpp"
#include <uuids.h>
#include <steam/isteamnetworkingutils.h>

namespace RavEngine {
	struct NetworkIdentity : public ComponentWithOwner, public Queryable<NetworkIdentity>{
	private:
        uuids::uuid NetworkID;
	public:
				
		//default constructor - used on Server (triggers spawn message)
        NetworkIdentity(entity_t owner);
		
		//Used on clients
		NetworkIdentity(entity_t owner, const uuids::uuid& id) : NetworkID(id),ComponentWithOwner(owner){
            // don't trigger spawn message
        }
		
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
