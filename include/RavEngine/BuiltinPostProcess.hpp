#pragma once
#include "PostProcess.hpp"

namespace RavEngine{

struct BloomEffect : public PostProcessEffect{
    BloomEffect();
};

struct BloomEffectInstance : public PostProcessEffectInstance{
    BloomEffectInstance(Ref<BloomEffect> effect) : PostProcessEffectInstance(effect){}
};

}
