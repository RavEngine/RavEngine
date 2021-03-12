#pragma once
#include "CTTI.hpp"

namespace RavEngine {
struct NetworkReplicable{
	
	virtual ctti_t NetTypeID() const = 0;
};
}
