#include "BuiltinPostProcess.hpp"
namespace RavEngine{

BloomEffect::BloomEffect() : PostProcessEffect("bloom", {
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
