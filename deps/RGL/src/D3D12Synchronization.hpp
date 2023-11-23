#pragma once
#define NOMINMAX
#include <RGL/Types.hpp>
#include <RGL/Synchronization.hpp>
#include "RGLD3D12.hpp"
#include <d3d12.h>

namespace RGL {
	struct DeviceD3D12;

	// implements a vulkan-style binary fence
	struct FenceD3D12 : public IFence {
		const std::shared_ptr<DeviceD3D12> owningDevice;
		HANDLE fenceEvent;

		FenceD3D12(decltype(owningDevice), bool preSignaled);

		Microsoft::WRL::ComPtr<ID3D12Fence> fence;

		// IFence
		void Wait() final;
		void Reset() final;
		void Signal() final;
		virtual ~FenceD3D12();
	};
}