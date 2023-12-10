#include "BuiltinPostProcess.hpp"
#include <RGL/RGL.hpp>
#include <RGL/Texture.hpp>
#include "App.hpp"
#include "RenderEngine.hpp"

namespace RavEngine{

constexpr static auto samplePassCount = 4;

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

BloomEffect::BloomEffect(){
    auto downsamplePass = New<BloomDownsamplePass>();
    auto upsamplePass = New<BloomUpsamplePass>();
    for(int i = 0; i < samplePassCount; i++){
        passes.push_back(New<BloomDownsamplePassInstance>(downsamplePass, i == 0));
    }
    for(int i = 0; i < samplePassCount; i++){
        passes.push_back(New<BloomUpsamplePassInstance>(upsamplePass, i == samplePassCount - 1));
    }
}

void BloomEffect::Preamble(dim_t<int> targetSize){
    if (tempTexture == nullptr || tempTexture->GetRHITexturePointer()->GetSize().width != targetSize.width ||  tempTexture->GetRHITexturePointer()->GetSize().height != targetSize.height){
        tempTexture = New<RuntimeTexture>(targetSize.width / 2, targetSize.height / 2, Texture::Config{
            .mipLevels = samplePassCount, 
            .enableRenderTarget = true,
            .format = RGL::TextureFormat::RGBA16_Sfloat,
            .debugName = "Bloom intermediate texture"
            }
        );
        
        // update texture handles
        auto underlying = tempTexture->GetRHITexturePointer();
        for(int i = 0; i < samplePassCount; i++){
            passes[i]->outputBinding = underlying->GetViewForMip(i);
            if (i != 0) {
                passes[i]->inputBindings[0] = underlying->GetViewForMip(i - 1);
            }
        }
        for(int i = samplePassCount; i < passes.size() - 1; i++){
            auto inMip = (samplePassCount - (i - samplePassCount)) - 1;
            auto outMip = inMip - 1;
            passes[i]->outputBinding = underlying->GetViewForMip(inMip);
            passes[i]->inputBindings[0] = underlying->GetViewForMip(outMip);
        }
        passes.back()->inputBindings[0] = underlying->GetViewForMip(0);
        passes.back()->outputBinding = {};
    }
}

}
