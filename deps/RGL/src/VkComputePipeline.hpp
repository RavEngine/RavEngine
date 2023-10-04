#pragma once
#include <RGL/Types.hpp>
#include <RGL/Pipeline.hpp>
#include "VkDevice.hpp"
#include "VkPipelineLibraryShared.hpp"

namespace RGL {
	struct ComputePipelineVk : public IComputePipeline {
		const std::shared_ptr<DeviceVk> owningDevice;
		const std::shared_ptr<struct PipelineLayoutVk> pipelineLayout;
		VkPipeline computePipeline = VK_NULL_HANDLE;

		BufferBindingStore bufferBindings;

		ComputePipelineVk(decltype(owningDevice) device, const ComputePipelineDescriptor& desc);

		virtual ~ComputePipelineVk();
	};
}
