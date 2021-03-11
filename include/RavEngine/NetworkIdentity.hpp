#pragma once
#include "Component.hpp"
#include "Queryable.hpp"
#include <uuids.h>

namespace RavEngine {
	struct NetworkIdentity : public Component, public Queryable< NetworkIdentity>{
	private:
        const uuids::uuid NetworkID;
	public:
		
		//default constructor - used on Server
		NetworkIdentity() : NetworkIdentity(uuids::uuid::create()){}
		
		//Used on clients
		NetworkIdentity(const uuids::uuid& id) : NetworkID(id){}
		
		inline decltype(NetworkID) GetNetworkID() const{
			return NetworkID;
		}
		
		enum class Ownership{
			Owner, NotOwner
		} ownership = Ownership::NotOwner;
	};
}
