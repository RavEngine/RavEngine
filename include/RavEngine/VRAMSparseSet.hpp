#pragma once
#include "SparseSet.hpp"
#include "VRAMVector.hpp"
#include <RGL/Types.hpp>

namespace RavEngine {

	template<typename T>
	class VRAMUnorderedVector : public unordered_vector<T, VRAMVector<T, false>> {};

	template<typename index_t, typename T>
	struct VRAMSparseSet : public UnorderedSparseSetGenericContainer<index_t, VRAMUnorderedVector<T>> {
		
	};
}
