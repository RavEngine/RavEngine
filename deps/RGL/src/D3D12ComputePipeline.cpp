#if RGL_DX12_AVAILABLE
#include "D3D12ComputePipeline.hpp"
#include "D3D12Device.hpp"
#include "RGLD3D12.hpp"
#include "D3D12RenderPipeline.hpp"
#include "D3D12ShaderLibrary.hpp"

namespace RGL {
	ComputePipelineD3D12::ComputePipelineD3D12(const decltype(owningDevice) owningDevice, const ComputePipelineDescriptor& desc) : owningDevice(owningDevice), pipelineLayout(std::static_pointer_cast<PipelineLayoutD3D12>(desc.pipelineLayout))
	{
		auto device = owningDevice->device;
		auto castedShader = std::static_pointer_cast<ShaderLibraryD3D12>(desc.stage.shaderModule);

		D3D12_COMPUTE_PIPELINE_STATE_DESC pipelineStateDesc{
			.pRootSignature = pipelineLayout->rootSignature.Get(),
			.CS = castedShader->shaderBytecode,
			.CachedPSO = nullptr,
		};

		DX_CHECK(device->CreateComputePipelineState(&pipelineStateDesc, IID_PPV_ARGS(&pipelineState)));
	}
	ComputePipelineD3D12::~ComputePipelineD3D12()
	{

	}
}
#endif