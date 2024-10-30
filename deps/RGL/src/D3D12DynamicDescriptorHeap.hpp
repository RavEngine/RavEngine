#pragma once
#include <memory>
#include <queue>
#include <directx/d3d12.h>
#include <DescriptorHeap.h>
#include "FreeList.hpp"


namespace RGL {
	struct D3D12DynamicDescriptorHeap : public DirectX::DescriptorHeap  {
		constexpr static auto totalCount = 2048u;
		using index_t = std::remove_const_t<std::make_unsigned_t<decltype(totalCount)>>;
	private:
		ID3D12Device* owningDevice;
		FreeList<index_t, totalCount> freeList;
	public:

		D3D12DynamicDescriptorHeap(decltype(owningDevice) device, D3D12_DESCRIPTOR_HEAP_TYPE type, D3D12_DESCRIPTOR_HEAP_FLAGS flags);

		/**
		* Allocate a single descriptor
		* @return Returns the offset into the descriptor heap.
		* We assume that a descriptor will be written to the index that is returned.
		*/
		index_t AllocateSingle();

		/**
		* Mark a descriptor as no longer needed. 
		* @param index the descriptor to deallocate
		*/
		void DeallocateSingle(index_t index);
		
	};
}
