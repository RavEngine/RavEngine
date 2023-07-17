#if RGL_VK_AVAILABLE
#include "VkRenderPipeline.hpp"
#include "RGLVk.hpp"
#include "VkShaderLibrary.hpp"
#include "VkBuffer.hpp"
#include "VkSampler.hpp"
#include "VkTexture.hpp"

namespace RGL {

    VkFrontFace RGL2VkFrontFace(RGL::WindingOrder windingOrder) {
        switch (windingOrder) {
        case decltype(windingOrder)::Clockwise: return VK_FRONT_FACE_CLOCKWISE;
        case decltype(windingOrder)::Counterclockwise: return VK_FRONT_FACE_COUNTER_CLOCKWISE;
        default:
            FatalError("Invalid cull mode");
        } 
    }

    VkPrimitiveTopology RGL2VkTopology(PrimitiveTopology top) {
        switch (top) {
            case decltype(top)::PointList: return VK_PRIMITIVE_TOPOLOGY_POINT_LIST;
            case decltype(top)::LineList: return VK_PRIMITIVE_TOPOLOGY_LINE_LIST;
            case decltype(top)::LineStrip: return VK_PRIMITIVE_TOPOLOGY_LINE_STRIP;
            case decltype(top)::TriangleList: return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
            case decltype(top)::TriangleStrip: return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP;
            case decltype(top)::TriangleFan: return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_FAN;
            case decltype(top)::LineListAdjacency: return VK_PRIMITIVE_TOPOLOGY_LINE_LIST_WITH_ADJACENCY;
            case decltype(top)::LineStripAdjacency: return VK_PRIMITIVE_TOPOLOGY_LINE_STRIP_WITH_ADJACENCY;
            case decltype(top)::TriangleListAdjacency: return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST_WITH_ADJACENCY;
            case decltype(top)::TriangleStripAdjacency: return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP_WITH_ADJACENCY;
            case decltype(top)::PatchList: return VK_PRIMITIVE_TOPOLOGY_PATCH_LIST;
        }
    }

    VkPolygonMode RGLK2VkPolygon(PolygonOverride override) {
        switch (override) {
            case decltype(override)::Fill: return VK_POLYGON_MODE_FILL;
            case decltype(override)::Line: return VK_POLYGON_MODE_LINE;
            case decltype(override)::Point: return VK_POLYGON_MODE_POINT;
        }
    }

    VkShaderStageFlags rgl2vkstageflags(RGL::StageVisibility stage) {
        VkShaderStageFlags retval = 0;
        if (stage & RGL::StageVisibility::Vertex) {
            retval |= VK_SHADER_STAGE_VERTEX_BIT;
        }
        if (stage & RGL::StageVisibility::Fragment) {
            retval |= VK_SHADER_STAGE_FRAGMENT_BIT;
        }
        if (stage & RGL::StageVisibility::Compute) {
            retval |= VK_SHADER_STAGE_COMPUTE_BIT;
        }
        return retval;
    }

	RenderPipelineVk::RenderPipelineVk(decltype(owningDevice) device, const RenderPipelineDescriptor& desc) : owningDevice(device), pipelineLayout(std::static_pointer_cast<PipelineLayoutVk>(desc.pipelineLayout))
	{
        std::vector<VkPipelineShaderStageCreateInfo> shaderStages;
        shaderStages.reserve(desc.stages.size());

        for (const auto& stage : desc.stages) {
            shaderStages.push_back(VkPipelineShaderStageCreateInfo{
                .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
                .stage = RGL2VKshader(stage.type),
                .module = std::static_pointer_cast<ShaderLibraryVk>(stage.shaderModule)->shaderModule,
                .pName = "main"
            });
        }

        // this allows for some minor tweaks to the pipeline object after it's created
        // at draw time, the values must be specified (required)
        std::vector<VkDynamicState> dynamicStates = {
            VK_DYNAMIC_STATE_VIEWPORT,
            VK_DYNAMIC_STATE_SCISSOR,
        };
        VkPipelineDynamicStateCreateInfo dynamicState{
            .sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
            .dynamicStateCount = static_cast<uint32_t>(dynamicStates.size()),
            .pDynamicStates = dynamicStates.data()
        };

        std::vector<VkVertexInputBindingDescription> bindingDescriptions;
        for (const auto& binding : desc.vertexConfig.vertexBindings) {
            bindingDescriptions.push_back({
                .binding = binding.binding,
                .stride = binding.stride,
                .inputRate = binding.inputRate == decltype(binding.inputRate)::Vertex ? VK_VERTEX_INPUT_RATE_VERTEX : VK_VERTEX_INPUT_RATE_INSTANCE
            });
        }

        std::vector<VkVertexInputAttributeDescription>attributeDescriptions;
        attributeDescriptions.reserve(desc.vertexConfig.attributeDescs.size());
        for (const auto& attribute : desc.vertexConfig.attributeDescs) {
            attributeDescriptions.push_back(VkVertexInputAttributeDescription{
                 .location = attribute.location,
                .binding = attribute.binding,
                .format = static_cast<VkFormat>(attribute.format),   // these use the same numeric values as VkFormat for convenience
                .offset = attribute.offset
             });
        }

        // vertex format
        VkPipelineVertexInputStateCreateInfo vertexInputInfo{
            .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
            .vertexBindingDescriptionCount = static_cast<uint32_t>(bindingDescriptions.size()),
            .pVertexBindingDescriptions = bindingDescriptions.data(),
            .vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size()),
            .pVertexAttributeDescriptions = attributeDescriptions.data(),
        };

        // trilist, tristrip, etc
        VkPipelineInputAssemblyStateCreateInfo inputAssembly{
            .sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
            .topology = RGL2VkTopology(desc.inputAssembly.topology),
            .primitiveRestartEnable = desc.inputAssembly.primitiveRestartEnabled      // for STRIP topology
        };

        // the viewport
#if 1
       VkViewport viewport{
            .x = desc.viewport.x,
            .y = desc.viewport.height - desc.viewport.y,    // this is reversed for the same reason as the comment below
            .width = desc.viewport.width,
            .height = -desc.viewport.height,            // this is negative to convert Vulkan to use a Y-up coordinate system like the other APIs
            .minDepth = desc.viewport.minDepth,
            .maxDepth = desc.viewport.maxDepth       
        };
#else
        VkViewport viewport{
            .x = desc.viewport.x,
            .y = desc.viewport.y,    // this is reversed for the same reason as the comment below
            .width = desc.viewport.width,
            .height = desc.viewport.height,            // this is negative to convert Vulkan to use a Y-up coordinate system like the other APIs
            .minDepth = desc.viewport.minDepth,
            .maxDepth = desc.viewport.maxDepth
        };
#endif

        // the scissor
        VkRect2D scissor{
            scissor.offset = {desc.scissor.offset.first, desc.scissor.offset.second},
            scissor.extent = {desc.scissor.extent.first, desc.scissor.extent.second}
        };
        // here's where we set the dynamic pipeline states
        VkPipelineViewportStateCreateInfo viewportState{
            .sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
            .viewportCount = 1,
            .pViewports = &viewport,
            .scissorCount = 1,
            .pScissors = &scissor       // arrays go here, but using multiple requires enabling a GPU feature
        };

        // fragment stage config
        VkPipelineRasterizationStateCreateInfo rasterizer{
            .sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
            .depthClampEnable = desc.rasterizerConfig.depthClampEnable,    // if set to true, fragments out of range will be clamped instead of clipped, which we rarely want (example: shadow volumes, no need for end caps)
            .rasterizerDiscardEnable = desc.rasterizerConfig.rasterizerDiscardEnable, // if true, output to the framebuffer is disabled
            .polygonMode = RGLK2VkPolygon(desc.rasterizerConfig.polygonOverride),        // lines, points, fill (anything other than fill requires a GPU feature)
            .cullMode = static_cast<VkCullModeFlags>(desc.rasterizerConfig.cullMode),      // front vs backface culling
            .frontFace = RGL2VkFrontFace(desc.rasterizerConfig.windingOrder),   // CW vs CCW 
            .depthBiasEnable = desc.rasterizerConfig.depthBias.enable,            // depth bias is useful for shadow maps
            .depthBiasConstantFactor = desc.rasterizerConfig.depthBias.constantFactor,    // the next 3 are optional
            .depthBiasClamp = desc.rasterizerConfig.depthBias.clamp,
            .depthBiasSlopeFactor = desc.rasterizerConfig.depthBias.slopeFactor,
            .lineWidth = 1.0f,                       // thickness > 1 requires the wideLines GPU feature
        };

        // a way to configure hardware anti aliasing
        // this only occurs along geometry edges
        VkPipelineMultisampleStateCreateInfo multisampling{
            .sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
            .rasterizationSamples = RGLMSA2VK(desc.multisampleConfig.sampleCount),
            .sampleShadingEnable = desc.multisampleConfig.sampleShadingEnabled,
            .minSampleShading = 1.0f,   // the rest are optional (TODO: support these)
            .pSampleMask = nullptr,
            .alphaToCoverageEnable = desc.multisampleConfig.alphaToCoverageEnabled,
            .alphaToOneEnable = desc.multisampleConfig.alphaToOneEnabled
        };

        std::vector<VkPipelineColorBlendAttachmentState> colorBlendAttachments;
        for (const auto& attachment : desc.colorBlendConfig.attachments) {
            colorBlendAttachments.push_back(decltype(colorBlendAttachments)::value_type{
                .blendEnable = attachment.blendEnabled,
                .srcColorBlendFactor = static_cast<VkBlendFactor>(attachment.sourceColorBlendFactor), //the next 6 are optional
                .dstColorBlendFactor = static_cast<VkBlendFactor>(attachment.destinationColorBlendFactor), //optional
                .colorBlendOp = static_cast<VkBlendOp>(attachment.colorBlendOperation), // Optional
                .srcAlphaBlendFactor = static_cast<VkBlendFactor>(attachment.sourceAlphaBlendFactor),
                .dstAlphaBlendFactor = static_cast<VkBlendFactor>(attachment.destinationAlphaBlendFactor),
                .alphaBlendOp = static_cast<VkBlendOp>(attachment.alphaBlendOperation),
                .colorWriteMask = static_cast<VkColorComponentFlags>(attachment.colorWriteMask),
            });
        }
        VkPipelineColorBlendStateCreateInfo colorBlending{
            .sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
            .logicOpEnable = desc.colorBlendConfig.logicalOpEnabled,
            .logicOp = static_cast<VkLogicOp>(desc.colorBlendConfig.logicalOperation),    //optional
            .attachmentCount = static_cast<uint32_t>(colorBlendAttachments.size()),           // for MRT
            .pAttachments = colorBlendAttachments.data(),  // specify all the attachments here
            .blendConstants = {desc.colorBlendConfig.blendconstants[0],desc.colorBlendConfig.blendconstants[1],desc.colorBlendConfig.blendconstants[2],desc.colorBlendConfig.blendconstants[3]},        // optional
        };

        const uint32_t nattachments = desc.colorBlendConfig.attachments.size();
        stackarray(attachmentFormats, VkFormat, nattachments);
        for (int i = 0; i < nattachments; i++) {
            attachmentFormats[i] = RGL2VkTextureFormat(desc.colorBlendConfig.attachments[i].format);
        }

        VkPipelineRenderingCreateInfoKHR renderingCreateInfo{
            .sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO,
            .pNext = VK_NULL_HANDLE,
            .colorAttachmentCount = nattachments,
            .pColorAttachmentFormats = attachmentFormats,
            .depthAttachmentFormat = RGL2VkTextureFormat(desc.depthStencilConfig.depthFormat),
            .stencilAttachmentFormat = RGL2VkTextureFormat(desc.depthStencilConfig.stencilFormat)
        };

        VkPipelineDepthStencilStateCreateInfo depthStencil{
            .sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,
            .depthTestEnable = desc.depthStencilConfig.depthTestEnabled,
            .depthWriteEnable = desc.depthStencilConfig.depthWriteEnabled,
            .depthCompareOp = static_cast<VkCompareOp>(desc.depthStencilConfig.depthFunction),
            .depthBoundsTestEnable = VK_FALSE,
            .stencilTestEnable = desc.depthStencilConfig.stencilTestEnabled,
            .front = static_cast<VkStencilOp>(desc.depthStencilConfig.stencilFrontOperation),
            .back = static_cast<VkStencilOp>(desc.depthStencilConfig.stencilBackOperation),
            .minDepthBounds = 0.0f,         // Optional,
            .maxDepthBounds = 1.0f,      // Optional
        };

        // create the pipeline object
        VkGraphicsPipelineCreateInfo pipelineInfo{
            .sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
            .pNext = &renderingCreateInfo,
            .stageCount = static_cast<uint32_t>(shaderStages.size()),
            .pStages = shaderStages.data(),
            .pVertexInputState = &vertexInputInfo,
            .pInputAssemblyState = &inputAssembly,
            .pViewportState = &viewportState,
            .pRasterizationState = &rasterizer,
            .pMultisampleState = &multisampling,
            .pDepthStencilState = &depthStencil,
            .pColorBlendState = &colorBlending,
            .pDynamicState = &dynamicState,
            .layout = pipelineLayout->layout,
            .renderPass = VK_NULL_HANDLE,       // VK_KHR_dynamic_rendering
            .subpass = 0,
            .basePipelineHandle = VK_NULL_HANDLE, // optional
            .basePipelineIndex = -1, // optional
            
        };
        VK_CHECK(vkCreateGraphicsPipelines(owningDevice->device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &graphicsPipeline));
	}
    RenderPipelineVk::~RenderPipelineVk()
    {
        vkDestroyPipeline(owningDevice->device, graphicsPipeline, nullptr);
    }

    PipelineLayoutVk::PipelineLayoutVk(decltype(owningDevice) device, const PipelineLayoutDescriptor& desc) : owningDevice(device)
    {
        std::vector<VkDescriptorSetLayoutBinding> layoutbindings;
        layoutbindings.reserve(desc.bindings.size());        

        for (const auto& binding : desc.bindings) {
            const auto type = static_cast<VkDescriptorType>(binding.type);
            const auto stageFlags = static_cast<VkShaderStageFlags>(binding.stageFlags);
            layoutbindings.push_back(
                VkDescriptorSetLayoutBinding {
                  .binding = binding.binding,   // see vertex shader
                  .descriptorType = type,
                  .descriptorCount = 1,
                  .stageFlags = stageFlags,
                  .pImmutableSamplers = nullptr
                }
            );
        }
      
        VkDescriptorSetLayoutCreateInfo layoutInfo{
            .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
            .flags = VK_DESCRIPTOR_SET_LAYOUT_CREATE_PUSH_DESCRIPTOR_BIT_KHR,   		// Setting this flag tells the descriptor set layouts that no actual descriptor sets are allocated but instead pushed at command buffer creation time
            .bindingCount = static_cast<uint32_t>(layoutbindings.size()),
            .pBindings = layoutbindings.data()
        };

        // create the descriptor set layout
        VK_CHECK(vkCreateDescriptorSetLayout(owningDevice->device, &layoutInfo, nullptr, &descriptorSetLayout));

        // setup push constants
        const auto nconstants = desc.constants.size();
        stackarray(pushconstants, VkPushConstantRange, nconstants);

        for (int i = 0; i < nconstants; i++) {
            const auto flags = rgl2vkstageflags(desc.constants[i].visibility);
            pushConstantBindingStageFlags[desc.constants[i].n_register] = flags;
            pushconstants[i].offset = desc.constants[i].n_register;
            pushconstants[i].size = desc.constants[i].size_bytes;
            pushconstants[i].stageFlags = flags;
        }


        VkPipelineLayoutCreateInfo pipelineLayoutInfo{
            .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
            .flags = 0,
            .setLayoutCount = 1,    // the rest are optional
            .pSetLayouts = &descriptorSetLayout,
            .pushConstantRangeCount = static_cast<uint32_t>(nconstants),
            .pPushConstantRanges = pushconstants
        };
        VK_CHECK(vkCreatePipelineLayout(owningDevice->device, &pipelineLayoutInfo, nullptr, &layout));
    }

    PipelineLayoutVk::~PipelineLayoutVk()
    {
        vkDestroyDescriptorSetLayout(owningDevice->device, descriptorSetLayout, nullptr);
        vkDestroyPipelineLayout(owningDevice->device, layout, nullptr);
    }
}

#endif