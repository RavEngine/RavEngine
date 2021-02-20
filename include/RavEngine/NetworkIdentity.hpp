#pragma once
#include "Component.hpp"
#include "Queryable.hpp"

namespace RavEngine {
	class NetworkIdentity : public Component, public Queryable< NetworkIdentity>{

	};
}