#pragma once
#include "PostProcess.hpp"

namespace RavEngine{

struct BloomDownsamplePass : public PostProcessPass{
    BloomDownsamplePass();
};

struct BloomUpsamplePass : public PostProcessPass{
    BloomUpsamplePass();
};


struct BloomDownsamplePassInstance : public PostProcessPassInstance{
    BloomDownsamplePassInstance(Ref<BloomDownsamplePass> effect) : PostProcessPassInstance(effect){}
};

struct BloomUpsamplePassInstance : public PostProcessPassInstance{
    BloomUpsamplePassInstance(Ref<BloomUpsamplePass> effect) : PostProcessPassInstance(effect){}
};

struct BloomEffect : public PostProcessEffect{
    BloomEffect();
    void Preamble(dim_t<int> targetSize) final;
private:
    Ref<Texture> tempTexture;
};

}
