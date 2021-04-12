#pragma once
#include "System.hpp"
#include "RPCComponent.hpp"

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

		const System::list_type& QueryTypes() const {
			return queries;
		}
	private:
		static const System::list_type queries;
	};
}
