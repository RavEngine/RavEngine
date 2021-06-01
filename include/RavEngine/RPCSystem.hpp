#pragma once
#include "System.hpp"
#include "RPCComponent.hpp"
#include "QueryIterator.hpp"

namespace RavEngine {
	class RPCSystem : public AutoCTTI{
	public:
		void Tick(float fpsScale, Ref<Component> c) {
            //id is always CTTI<RPCComponent>()
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
