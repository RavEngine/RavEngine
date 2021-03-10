#pragma once
#include <phmap.h>
#include "CTTI.hpp"
#include <unordered_map>

namespace RavEngine {
struct NetworkReplicable{
	
	virtual ctti_t NetTypeID() const = 0;
};
}
