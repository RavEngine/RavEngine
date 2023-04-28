#if RGL_DX12_AVAILABLE
#include "D3D12Synchronization.hpp"
#include "D3D12Device.hpp"
#include "D3D12CommandQueue.hpp"

namespace RGL {
	FenceD3D12::FenceD3D12(decltype(owningDevice) device, bool preSignaled) : owningDevice(device)
	{
		DX_CHECK(owningDevice->device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence)));
		fenceEvent = ::CreateEvent(NULL, FALSE, FALSE, NULL);
		if (preSignaled) {
			fence->Signal(1);
		}
	}
	void FenceD3D12::Wait()
	{
		if (fence->GetCompletedValue() != 1) {
			fence->SetEventOnCompletion(1, fenceEvent);

			::WaitForSingleObject(fenceEvent, DWORD_MAX);
		}
	}
	void FenceD3D12::Reset()
	{
		owningDevice->internalQueue->GetD3D12CommandQueue()->Signal(fence.Get(), 0);
	}
	void FenceD3D12::Signal()
	{
		owningDevice->internalQueue->GetD3D12CommandQueue()->Signal(fence.Get(), 1);
	}
}

#endif