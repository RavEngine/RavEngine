#pragma once
#include "System.hpp"
#include "RPCComponent.hpp"

namespace RavEngine {
	class RPCSystem {
	public:
		void Tick(float fpsScale, Ref<Entity> e) {
			auto rpcs = e->GetAllComponentsOfType<RPCComponent>();
			for (const auto& r : rpcs) {
				auto rpc = std::static_pointer_cast<RPCComponent>(r);
				rpc->Swap();
				rpc->ProcessClientRPCs();
				rpc->ProcessServerRPCs();
			}
		}

		const System::list_type& QueryTypes() const {
			return queries;
		}
	private:
		static const System::list_type queries;
	};
}