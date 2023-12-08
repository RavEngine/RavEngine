#include "BuiltinPostProcess.hpp"
namespace RavEngine{

BloomDownsamplePass::BloomDownsamplePass() : PostProcessPass("bloom_downsample", {
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

BloomUpsamplePass::BloomUpsamplePass() : PostProcessPass("bloom_downsample", {
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

BloomEffect::BloomEffect(){
    const auto samplePassCount = 4;
    auto downsamplePass = New<BloomDownsamplePass>();
    auto upsamplePass = New<BloomUpsamplePass>();
    for(int i = 0; i < samplePassCount; i++){
        passes.push_back(New<BloomDownsamplePassInstance>(downsamplePass));
    }
    for(int i = 0; i < samplePassCount; i++){
        passes.push_back(New<BloomUpsamplePassInstance>(upsamplePass));
    }
}

void BloomEffect::Preamble(dim_t<int> targetSize){
    auto mipLevels = std::log2(std::max(targetSize.width, targetSize.height));
    tempTexture = New<RuntimeTexture>(targetSize.width, targetSize.height, mipLevels, 1, nullptr);
}

}
