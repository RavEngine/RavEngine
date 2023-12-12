#pragma once
#include "PostProcess.hpp"

namespace RavEngine{

struct BloomEffect : public PostProcessEffect{
    BloomEffect();
    void Preamble(dim_t<int> targetSize) final;
private:
    struct UpsampleConstants {
        float radius = 0.005f;
    };
    struct ApplyConstants {
        float bloomPower = 0.04;
    };
    struct BloomDownsamplePass : public PostProcessPass{
        BloomDownsamplePass();
    };

    struct BloomUpsamplePass : public PostProcessPass{
        
        BloomUpsamplePass();
    };

    struct BloomApplyPass : public PostProcessPass {
        BloomApplyPass();
    };

    struct BloomDownsamplePassInstance : public PostProcessPassInstance{
        dim_t<int> outputSize;
        BloomDownsamplePassInstance(Ref<BloomDownsamplePass> effect, bool isFirst);
        dim_t<int> GetUserDefinedOutputSize() const final {
            return outputSize;
        }
    };

    struct BloomUpsamplePassInstance : public PostProcessPassInstance{
        dim_t<int> outputSize;
        BloomUpsamplePassInstance(Ref<BloomUpsamplePass> effect, bool isLast);
        UpsampleConstants pushConstants;
        const RGL::untyped_span GetPushConstantData() const final{
            return pushConstants;    
        }
        dim_t<int> GetUserDefinedOutputSize() const final {
            return outputSize;
        }
    };

    struct BloomApplyPassInstance : public PostProcessPassInstance {
        ApplyConstants pushConstants;
        const RGL::untyped_span GetPushConstantData() const final {
            return pushConstants;
        }
        BloomApplyPassInstance(Ref<BloomApplyPass> effect);
    };
    
    RGLSamplerPtr sampler;
    Ref<Texture> tempTexture;
    Ref<BloomDownsamplePass> downsamplePass;
    Ref<BloomUpsamplePass> upsamplePass;
    Ref<BloomApplyPass> applyPass;
};

}
