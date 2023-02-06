#pragma once

#include "ScriptComponent.hpp"
#include "CTTI.hpp"

namespace RavEngine {
	class ScriptSystem : public AutoCTTI {
	public:
         void operator()(ScriptComponent& c) const;
	};
}
