#pragma once
#include "System.hpp"
#include "RPCComponent.hpp"
#include "QueryIterator.hpp"

namespace RavEngine {
	class RPCSystem : public AutoCTTI{
	public:
		void Tick(float fpsScale, Ref<RPCComponent> rpc){
            rpc->Swap();
            rpc->ProcessClientRPCs();
            rpc->ProcessServerRPCs();
		}

		constexpr QueryIteratorAND<RPCComponent> QueryTypes() const {
			return QueryIteratorAND<RPCComponent>();
		}
	};
}
