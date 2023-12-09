#pragma once
#include "PostProcess.hpp"

namespace RavEngine{

struct BloomEffect : public PostProcessEffect{
    BloomEffect();
    void Preamble(dim_t<int> targetSize) final;
private:
    struct BloomDownsamplePass : public PostProcessPass{
        BloomDownsamplePass();
    };

    struct BloomUpsamplePass : public PostProcessPass{
        BloomUpsamplePass();
    };


    struct BloomDownsamplePassInstance : public PostProcessPassInstance{
        BloomDownsamplePassInstance(Ref<BloomDownsamplePass> effect, bool isFirst);
    };

    struct BloomUpsamplePassInstance : public PostProcessPassInstance{
        BloomUpsamplePassInstance(Ref<BloomUpsamplePass> effect);
    };
    
    Ref<Texture> tempTexture;
};

}
