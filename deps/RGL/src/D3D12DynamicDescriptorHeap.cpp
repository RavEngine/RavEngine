#if RGL_DX12_AVAILABLE
#include "D3D12DynamicDescriptorHeap.hpp"
#include <DescriptorHeap.h>
#include <cassert>

namespace RGL {
	D3D12DynamicDescriptorHeap::D3D12DynamicDescriptorHeap(decltype(owningDevice) device, D3D12_DESCRIPTOR_HEAP_TYPE type, D3D12_DESCRIPTOR_HEAP_FLAGS flags) : owningDevice(device), DescriptorHeap(device,type,flags, totalCount)
	{
		// start with 1 pile
	}
	D3D12DynamicDescriptorHeap::index_t D3D12DynamicDescriptorHeap::AllocateSingle()
	{
		index_t allocatedIndex = 0;
		if (freeList.empty()) {
			if (nextFreeIndexNotInQueue == totalCount) {
				// full -- cannot allocate!
				throw std::out_of_range("Descriptor heap is full!");
			}
			// place the descriptor at the end
			allocatedIndex = nextFreeIndexNotInQueue;
			nextFreeIndexNotInQueue++;
		}
		else {
			// fill the hole
			allocatedIndex = freeList.front();
			freeList.pop();
		}

		return allocatedIndex;
	}

	void D3D12DynamicDescriptorHeap::DeallocateSingle(index_t index)
	{
		// if the index was the end, then decrement end
		if (index == nextFreeIndexNotInQueue) {
			nextFreeIndexNotInQueue--;
		}
		else {
			// add the index to the free list
			freeList.emplace(index);
		}
	}

}
#endif