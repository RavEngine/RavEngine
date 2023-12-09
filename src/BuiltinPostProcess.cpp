#include "BuiltinPostProcess.hpp"
#include <RGL/RGL.hpp>
#include <RGL/Texture.hpp>
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
    .output = PostProcessOutput::UserDefined
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
    .output = PostProcessOutput::UserDefined
}){}

BloomEffect::BloomDownsamplePassInstance::BloomDownsamplePassInstance(Ref<BloomDownsamplePass> effect, bool isFirst) :  PostProcessPassInstance(effect){
    inputConfiguration = isFirst ? InputConfigurationType{PostProcessTextureInput::EngineColor} : InputConfigurationType{PostProcessTextureInput::UserDefined};
}

BloomEffect::BloomUpsamplePassInstance::BloomUpsamplePassInstance::BloomUpsamplePassInstance(Ref<BloomUpsamplePass> effect) : PostProcessPassInstance(effect){
    inputConfiguration = InputConfigurationType{PostProcessTextureInput::UserDefined};
}

BloomEffect::BloomEffect(){
    auto downsamplePass = New<BloomDownsamplePass>();
    auto upsamplePass = New<BloomUpsamplePass>();
    for(int i = 0; i < samplePassCount; i++){
        passes.push_back(New<BloomDownsamplePassInstance>(downsamplePass, i == 0));
    }
    for(int i = 0; i < samplePassCount; i++){
        passes.push_back(New<BloomUpsamplePassInstance>(upsamplePass));
    }
}

void BloomEffect::Preamble(dim_t<int> targetSize){
    if (tempTexture == nullptr || tempTexture->GetRHITexturePointer()->GetSize().width != targetSize.width ||  tempTexture->GetRHITexturePointer()->GetSize().height != targetSize.height){
        tempTexture = New<RuntimeTexture>(targetSize.width / 2, targetSize.height / 2, samplePassCount, 1, nullptr);
        
        // update texture handles
        auto underlying = tempTexture->GetRHITexturePointer();
        for(int i = 0; i < samplePassCount; i++){
            //passes[i]->outputBinding = std::move(std::make_unique<RGL::TextureView,BloomDownsamplePassInstance::TextureBindingPtrDeleterType>(underlying->GetViewForMip(i+1), [](auto*){}));
        }
        for(int i = samplePassCount; i < passes.size() - 1; i++){
            //passes[i]->outputBinding = std::move(std::make_unique<RGL::TextureView,BloomDownsamplePassInstance::TextureBindingPtrDeleterType>(underlying->GetViewForMip(samplePassCount - i), [](auto*){}));
        }
        passes.back()->outputBinding = nullptr;
    }
}

}
