#pragma once
#include "SparseSet.hpp"
#include "VRAMVector.hpp"
#include <RGL/Types.hpp>

namespace RavEngine {

	template<typename T>
	class VRAMUnorderedVector : public unordered_vector<T, VRAMVector<T, false>> {};

	struct VRAMSparseSet_ImplementationDetails {
		RGLDevicePtr GetDevice();
	};

	template<typename index_t, typename T>
	struct VRAMSparseSet : public UnorderedSparseSetGenericContainer<index_t, VRAMUnorderedVector<T>>, public VRAMSparseSet_ImplementationDetails {
		
		VRAMSparseSet() {
			UnorderedSparseSetGenericContainer<index_t, VRAMUnorderedVector<T>>::GetDense().get_underlying().InitialSetup(GetDevice());
		}
	};
}