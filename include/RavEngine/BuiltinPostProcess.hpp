#pragma once
#include "PostProcess.hpp"

namespace RavEngine{

struct BloomEffect : public PostProcessEffect{
    BloomEffect();
    void Preamble(dim_t<int> targetSize) final;
private:
    struct UpsampleConstants {
        float radius = 3;
    };
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
        BloomUpsamplePassInstance(Ref<BloomUpsamplePass> effect, bool isLast);
        UpsampleConstants pushConstants;
        const RGL::untyped_span GetPushConstantData() const final{
            return pushConstants;    
        }
    };
    
    Ref<Texture> tempTexture;
};

}
