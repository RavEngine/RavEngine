#pragma once
#include "Component.hpp"
#include "Queryable.hpp"
#include <uuids.h>

namespace RavEngine {
	class NetworkIdentity : public Component, public Queryable< NetworkIdentity>{
        const uuids::uuid NetworkID = uuids::uuid::create();
        
	};
}
