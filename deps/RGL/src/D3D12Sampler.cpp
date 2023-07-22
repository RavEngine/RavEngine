#if RGL_DX12_AVAILABLE
#include "D3D12Sampler.hpp"
#include "RGLD3D12.hpp"
#include "D3D12Device.hpp"
#include <limits>
#undef max

namespace RGL {

	D3D12_TEXTURE_ADDRESS_MODE rgl2d3d12addressmode(SamplerAddressMode mode) {
		switch (mode) {
		case decltype(mode)::Wrap: return D3D12_TEXTURE_ADDRESS_MODE_WRAP;
		case decltype(mode)::Mirror: return D3D12_TEXTURE_ADDRESS_MODE_MIRROR;
		case decltype(mode)::Clamp: return D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
		case decltype(mode)::Border: return D3D12_TEXTURE_ADDRESS_MODE_BORDER;
		case decltype(mode)::MirrorOnce: return D3D12_TEXTURE_ADDRESS_MODE_MIRROR_ONCE;
		}
	}

	SamplerD3D12::SamplerD3D12(decltype(owningDevice) owningDevice, const SamplerConfig& config) : owningDevice(owningDevice),
		descriptorIndex(owningDevice->SamplerHeap->AllocateSingle())
	{

		auto handle = owningDevice->SamplerHeap->GetCpuHandle(descriptorIndex);

		auto filter = D3D12_FILTER_ANISOTROPIC;
		if (config.compareFunction != DepthCompareFunction::Always) {
			filter = D3D12_FILTER_COMPARISON_ANISOTROPIC;
		}

		 samplerDesc = {
			.Filter = filter,
			.AddressU = rgl2d3d12addressmode(config.addressModeU),
			.AddressV = rgl2d3d12addressmode(config.addressModeV),
			.AddressW = rgl2d3d12addressmode(config.addressModeW),
			.MipLODBias = 0,
			.MaxAnisotropy = 1,
			.ComparisonFunc = rgl2d3dcompfn(config.compareFunction),
			.BorderColor = {config.borderColor[0],config.borderColor[1],config.borderColor[2],config.borderColor[3]},
			.MinLOD = 0,
			.MaxLOD = std::numeric_limits<decltype(samplerDesc.MaxLOD)>::max(),

		};

		owningDevice->device->CreateSampler(&samplerDesc, handle);
	}
	SamplerD3D12::~SamplerD3D12()
	{
		owningDevice->SamplerHeap->DeallocateSingle(descriptorIndex);
	}
}
#endif