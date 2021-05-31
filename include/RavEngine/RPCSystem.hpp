#pragma once
#include "System.hpp"
#include "RPCComponent.hpp"
#include "QueryIterator.hpp"

namespace RavEngine {
	class RPCSystem : public AutoCTTI{
	public:
		void Tick(float fpsScale, Ref<Component> c, ctti_t id) {
            //id is always CTTI<RPCComponent>()
            Debug::Assert(id == CTTI<RPCComponent>(), "RPC system passed component of incorrect type!");
            auto rpc = std::static_pointer_cast<RPCComponent>(c);
            rpc->Swap();
            rpc->ProcessClientRPCs();
            rpc->ProcessServerRPCs();
		}

		constexpr QueryIteratorAND<RPCComponent> QueryTypes() const {
			return QueryIteratorAND<RPCComponent>();
		}
	};
}
