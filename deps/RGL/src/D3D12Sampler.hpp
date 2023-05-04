#pragma once
#define NOMINMAX
#include <RGL/Types.hpp>
#include <RGL/Sampler.hpp>
#include "RGLD3D12.hpp"
#include <memory>

namespace RGL {
	struct DeviceD3D12;
	struct SamplerD3D12 : public ISampler {
		const std::shared_ptr<DeviceD3D12> owningDevice;
		const UINT descriptorIndex;

		SamplerD3D12(decltype(owningDevice), const SamplerConfig&);
		virtual ~SamplerD3D12();

		D3D12_SAMPLER_DESC samplerDesc;
	};
}