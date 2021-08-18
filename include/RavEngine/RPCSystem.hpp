#pragma once
#include "System.hpp"
#include "RPCComponent.hpp"

namespace RavEngine {
	class RPCSystem : public AutoCTTI{
	public:
		inline void Tick(float fpsScale, Ref<RPCComponent> rpc)const{
            rpc->Swap();
            rpc->ProcessClientRPCs();
            rpc->ProcessServerRPCs();
		}
	};
}
