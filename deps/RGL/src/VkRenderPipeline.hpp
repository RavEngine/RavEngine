#pragma once
#include <RGL/Types.hpp>
#include <RGL/Pipeline.hpp>
#include "VkDevice.hpp"
#include "VkRenderPass.hpp"
#include <unordered_map>

namespace RGL {

	struct PipelineLayoutVk : public IPipelineLayout {
		const std::shared_ptr<DeviceVk> owningDevice;
		VkPipelineLayout layout = VK_NULL_HANDLE;
		VkDescriptorSetLayout descriptorSetLayout = VK_NULL_HANDLE;

		std::unordered_map<uint32_t, VkShaderStageFlags> pushConstantBindingStageFlags;

		PipelineLayoutVk(decltype(owningDevice), const PipelineLayoutDescriptor&);
		virtual ~PipelineLayoutVk();
	};

	struct RenderPipelineVk : public IRenderPipeline {
		const std::shared_ptr<DeviceVk> owningDevice;
		const std::shared_ptr<PipelineLayoutVk> pipelineLayout;
		VkPipeline graphicsPipeline = VK_NULL_HANDLE;

		RenderPipelineVk(decltype(owningDevice), const RenderPipelineDescriptor&);
		virtual ~RenderPipelineVk();
	};

	
}