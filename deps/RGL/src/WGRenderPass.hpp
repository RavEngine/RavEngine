#pragma once
#include <RGL/RenderPass.hpp>
#include <array>
#include <emscripten/html5_webgpu.h>

namespace RGL{
    struct TextureView;
    
    struct RenderPassWG : public IRenderPass{

        WGPURenderPassDescriptorMaxDrawCount maxDrawCount{
            .chain = {
                .next = nullptr,
                .sType = WGPUSType_RenderPassDescriptorMaxDrawCount
            },
            .maxDrawCount = 65535
        };
        WGPURenderPassDescriptor renderPass;
        std::vector<WGPURenderPassColorAttachment> colorAttachments;
        WGPURenderPassDepthStencilAttachment depthStencilAttachment;

        // IRenderPass
        RenderPassWG(const RenderPassConfig& config);
        virtual ~RenderPassWG(){}
        void SetAttachmentTexture(uint32_t index, const TextureView& texture) final;
        
        void SetDepthAttachmentTexture(const TextureView& texture) final;
        void SetStencilAttachmentTexture(const TextureView& texture) final;
    };
}
