#pragma once
#include <RGL/Pipeline.hpp>
#include <RGL/Types.hpp>
#include <d3d12.h>
#include <directx/d3dx12.h>
#include "RGLD3D12.hpp"

namespace RGL {
	struct DeviceD3D12;
	struct PipelineLayoutD3D12;
	struct ComputePipelineD3D12 : public IComputePipeline {
		const std::shared_ptr<DeviceD3D12> owningDevice;
		const std::shared_ptr<PipelineLayoutD3D12> pipelineLayout;
		ComPtr<ID3D12PipelineState> pipelineState;
		ComputePipelineD3D12(const decltype(owningDevice) owningDevice, const ComputePipelineDescriptor& desc);

		virtual ~ComputePipelineD3D12();
	};
}