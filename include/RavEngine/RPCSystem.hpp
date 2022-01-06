#pragma once
#include "RPCComponent.hpp"

namespace RavEngine {
	class RPCSystem : public AutoCTTI{
	public:
		inline void operator()(float, RPCComponent& rpc)const{
            rpc.Swap();
            rpc.ProcessClientRPCs();
            rpc.ProcessServerRPCs();
		}
	};
}
