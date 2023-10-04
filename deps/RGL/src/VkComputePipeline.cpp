#if RGL_VK_AVAILABLE
#include "VkComputePipeline.hpp"
#include "VkRenderPipeline.hpp"
#include "VkShaderLibrary.hpp"
#include "RGLVk.hpp"

namespace RGL {
	ComputePipelineVk::ComputePipelineVk(decltype(owningDevice) owningDevice, const ComputePipelineDescriptor& desc) : owningDevice(owningDevice), pipelineLayout(std::static_pointer_cast<PipelineLayoutVk>(desc.pipelineLayout))
	{

        auto library = std::static_pointer_cast<ShaderLibraryVk>(desc.stage.shaderModule);
        VkPipelineShaderStageCreateInfo shaderStages{
            .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
            .stage = RGL2VKshader(desc.stage.type),
            .module = library->shaderModule,
            .pName = "main"
        };

        VkComputePipelineCreateInfo pipelineInfo{
            .sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO,
            .stage = shaderStages,
            .layout = pipelineLayout->layout,
        };

        bufferBindings = library->bindingInfo;

        VK_CHECK(vkCreateComputePipelines(owningDevice->device, VK_NULL_HANDLE, 1,&pipelineInfo,nullptr,&computePipeline));
	}
	ComputePipelineVk::~ComputePipelineVk()
	{
		vkDestroyPipeline(owningDevice->device, computePipeline, nullptr);
	}
}
#endif