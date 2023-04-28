#if RGL_VK_AVAILABLE
#include "VkComputePipeline.hpp"
#include "VkRenderPipeline.hpp"
#include "VkShaderLibrary.hpp"
#include "RGLVk.hpp"

namespace RGL {
	ComputePipelineVk::ComputePipelineVk(decltype(owningDevice) owningDevice, const ComputePipelineDescriptor& desc) : owningDevice(owningDevice), pipelineLayout(std::static_pointer_cast<PipelineLayoutVk>(desc.pipelineLayout))
	{
        VkPipelineShaderStageCreateInfo shaderStages{
            .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
            .stage = RGL2VKshader(desc.stage.type),
            .module = std::static_pointer_cast<ShaderLibraryVk>(desc.stage.shaderModule)->shaderModule,
            .pName = "main"
        };

        VkComputePipelineCreateInfo pipelineInfo{
            .sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO,
            .stage = shaderStages,
            .layout = pipelineLayout->layout,
        };

        VK_CHECK(vkCreateComputePipelines(owningDevice->device, VK_NULL_HANDLE, 1,&pipelineInfo,nullptr,&computePipeline));
	}
	ComputePipelineVk::~ComputePipelineVk()
	{
		vkDestroyPipeline(owningDevice->device, computePipeline, nullptr);
	}
}
#endif