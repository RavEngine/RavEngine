#pragma once
#include "ComponentWithOwner.hpp"
#include "Queryable.hpp"
#include "Uuid.hpp"
#include <steam/isteamnetworkingutils.h>

namespace RavEngine {
	struct NetworkIdentity : public ComponentWithOwner, public Queryable<NetworkIdentity>{
	private:
        uuids::uuid NetworkID;
		ctti_t NetTypeID = 0;		// the CTTI type of the T when it was constructed via CreatePrototype<T>, on clients this is set to 0
	public:
				
		//default constructor - used on Server (triggers spawn message)
		NetworkIdentity(entity_t owner, ctti_t ent_type_id) : NetworkID(uuids::uuid::create()), NetTypeID(ent_type_id), ComponentWithOwner(owner) {}
		
		//Used on clients
		NetworkIdentity(entity_t owner, const uuids::uuid& id) : NetworkID(id),ComponentWithOwner(owner){
            // don't trigger spawn message
        }
		
        inline const decltype(NetworkID)& GetNetworkID() const{
			return NetworkID;
		}

		inline decltype(NetTypeID) GetNetTypeID() const {
			return NetTypeID;
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
