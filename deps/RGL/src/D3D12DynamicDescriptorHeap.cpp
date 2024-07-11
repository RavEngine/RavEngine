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
		return freeList.Allocate();
	}

	void D3D12DynamicDescriptorHeap::DeallocateSingle(index_t index)
	{
		freeList.Deallocate(index);
	}

}
#endif