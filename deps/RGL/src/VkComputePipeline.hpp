#pragma once
#include <RGL/Types.hpp>
#include <RGL/Pipeline.hpp>
#include "VkDevice.hpp"

namespace RGL {
	struct ComputePipelineVk : public IComputePipeline {
		const std::shared_ptr<DeviceVk> owningDevice;
		const std::shared_ptr<struct PipelineLayoutVk> pipelineLayout;
		VkPipeline computePipeline = VK_NULL_HANDLE;

		ComputePipelineVk(decltype(owningDevice) device, const ComputePipelineDescriptor& desc);

		virtual ~ComputePipelineVk();
	};
}
