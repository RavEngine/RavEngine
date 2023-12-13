#if !RVE_SERVER
#include "BuiltinPostProcess.hpp"
#include <RGL/RGL.hpp>
#include <RGL/Texture.hpp>
#include <RGL/Sampler.hpp>
#include "App.hpp"
#include "RenderEngine.hpp"

namespace RavEngine{

BloomEffect::BloomDownsamplePass::BloomDownsamplePass() : PostProcessPass("bloom_downsample", {
    .bindings = {
        {
            .binding = 0,
            .type = RGL::BindingType::SampledImage,
            .stageFlags = RGL::BindingVisibility::Fragment,
        },
        {
            .binding = 1,
            .type = RGL::BindingType::Sampler,
            .stageFlags = RGL::BindingVisibility::Fragment,
        },
    },
}){}

BloomEffect::BloomUpsamplePass::BloomUpsamplePass() : PostProcessPass("bloom_upsample", {
    .bindings = {
        {
            .binding = 0,
            .type = RGL::BindingType::SampledImage,
            .stageFlags = RGL::BindingVisibility::Fragment,
        },
        {
            .binding = 1,
            .type = RGL::BindingType::Sampler,
            .stageFlags = RGL::BindingVisibility::Fragment,
        },
    },
    .pushConstantSize = sizeof(UpsampleConstants),
    .sourceColorBlendFactor = RGL::BlendFactor::One,
    .destinationColorBlendFactor = RGL::BlendFactor::One,
}){}

BloomEffect::BloomApplyPass::BloomApplyPass() : PostProcessPass("bloom_merge", {
    .bindings = {
        {
            .binding = 0,
            .type = RGL::BindingType::SampledImage,
            .stageFlags = RGL::BindingVisibility::Fragment,
        },
        {
            .binding = 1,
            .type = RGL::BindingType::SampledImage,
            .stageFlags = RGL::BindingVisibility::Fragment,
        },
        {
            .binding = 2,
            .type = RGL::BindingType::Sampler,
            .stageFlags = RGL::BindingVisibility::Fragment,
        },
    },
    .pushConstantSize = sizeof(ApplyConstants),
    }){}

BloomEffect::BloomDownsamplePassInstance::BloomDownsamplePassInstance(Ref<BloomDownsamplePass> effect, bool isFirst) : PostProcessPassInstance(effect, {
    .inputconfiguration = isFirst ? InputConfigurationType{PostProcessTextureInput::EngineColor} : InputConfigurationType{PostProcessTextureInput::UserDefined},
    .outputConfiguration = PostProcessOutput::UserDefined
    }) {}

BloomEffect::BloomUpsamplePassInstance::BloomUpsamplePassInstance::BloomUpsamplePassInstance(Ref<BloomUpsamplePass> effect, bool isLast) : PostProcessPassInstance(effect, {
    .inputconfiguration = InputConfigurationType{PostProcessTextureInput::UserDefined},
    .outputConfiguration = isLast ? PostProcessOutput::EngineColor : PostProcessOutput::UserDefined
    }) {
}

RavEngine::BloomEffect::BloomApplyPassInstance::BloomApplyPassInstance(Ref<BloomApplyPass> effect) : PostProcessPassInstance(effect,
    {
        .inputconfiguration = {PostProcessTextureInput::EngineColor, PostProcessTextureInput::UserDefined}
    }){}

BloomEffect::BloomEffect(){
    sampler = GetApp()->GetDevice()->CreateSampler({
        .addressModeU = RGL::SamplerAddressMode::Clamp,
        .addressModeV = RGL::SamplerAddressMode::Clamp,
        .addressModeW = RGL::SamplerAddressMode::Clamp,
    });

    downsamplePass = New<BloomDownsamplePass>();
    upsamplePass = New<BloomUpsamplePass>();
    applyPass = New<BloomApplyPass>();
}

void BloomEffect::Preamble(dim_t<int> targetSize){
    bool needsNewTexture = tempTexture == nullptr;
    
    if (tempTexture != nullptr) {
        auto oldSize = tempTexture->GetRHITexturePointer()->GetSize();
        if (oldSize.width != targetSize.width / 2 || oldSize.height != targetSize.height / 2) {
            needsNewTexture = true;
        }
    }

    if (needsNewTexture){
        passes.clear();

        uint32_t samplePassCount = std::log2(std::min(targetSize.width / 2, targetSize.height / 2));

        for (int i = 0; i < samplePassCount; i++) {
            auto instance = New<BloomDownsamplePassInstance>(downsamplePass, i == 0);
            instance->inputSamplerBindings[1] = sampler;
            passes.push_back(instance);
        }
        for (int i = 0; i < samplePassCount - 1; i++) {
            auto instance = New<BloomUpsamplePassInstance>(upsamplePass, i == samplePassCount - 1);
            passes.push_back(instance);
            instance->inputSamplerBindings[1] = sampler;
        }

        auto applyPassInstance = New<BloomApplyPassInstance>(applyPass);
        applyPassInstance->inputSamplerBindings[2] = sampler;
        passes.push_back(applyPassInstance);

        // make sure the intermediate texture is reset
        passes.front()->clearOutputBeforeRendering = true;

        dim_t<float> size{ targetSize.width / 2.0f, targetSize.height / 2.0f };
        tempTexture = New<RuntimeTexture>(size.width, size.height, Texture::Config{
            .mipLevels = uint8_t(samplePassCount), 
            .enableRenderTarget = true,
            .format = RGL::TextureFormat::RGBA16_Sfloat,
            .debugName = "Bloom intermediate texture"
            }
        );
        
        // update texture handles
        auto underlying = tempTexture->GetRHITexturePointer();
        for (int i = 0; i < samplePassCount; i++) {
            passes[i]->outputBinding = underlying->GetViewForMip(i);
            std::static_pointer_cast<BloomDownsamplePassInstance>(passes[i])->outputSize = { int(size.width), int(size.height) };
            if (i != 0) {
                passes[i]->inputBindings[0] = underlying->GetViewForMip(i - 1);
            }
            size.width /= 2; size.height /= 2;
        }
        size.width *= 2; size.height *= 2;
        for(int i = samplePassCount; i < passes.size() - 1; i++){
            auto inMip = (samplePassCount - (i - samplePassCount)) - 1;
            auto outMip = inMip - 1;
            passes[i]->inputBindings[0] = underlying->GetViewForMip(inMip);
            passes[i]->outputBinding = underlying->GetViewForMip(outMip);

            size.width *= 2; size.height *= 2;

            std::static_pointer_cast<BloomDownsamplePassInstance>(passes[i])->outputSize = { int(size.width), int(size.height) };
        }
        passes.back()->inputBindings[1] = underlying->GetViewForMip(0); // EngineColor is input[0] 
        passes.back()->outputBinding = {};
    }
}

FXAAEffect::FXAAEffect()
{
    auto pass = New<FXAAPass>();
    auto instance = New<FXAAPassInstance>(pass);
    passes.push_back(instance);

    auto sampler = GetApp()->GetDevice()->CreateSampler({
        .addressModeU = RGL::SamplerAddressMode::Clamp,
        .addressModeV = RGL::SamplerAddressMode::Clamp,
        .addressModeW = RGL::SamplerAddressMode::Clamp,
    });

    instance->inputSamplerBindings[1] = sampler;
}

FXAAEffect::FXAAPass::FXAAPass() : PostProcessPass("fxaa", {
    .bindings = {
        {
            .binding = 0,
            .type = RGL::BindingType::SampledImage,
            .stageFlags = RGL::BindingVisibility::Fragment,
        },
        {
            .binding = 1,
            .type = RGL::BindingType::Sampler,
            .stageFlags = RGL::BindingVisibility::Fragment,
        },
    },
}){}

}
#endif
