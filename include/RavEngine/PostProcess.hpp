#pragma once
#if !RVE_SERVER
#include <string_view>
#include <RGL/Pipeline.hpp>
#include "Ref.hpp"
#include "DataStructures.hpp"
#include "mathtypes.hpp"
#include "Texture.hpp"
#include <RGL/Span.hpp>
#include <RGL/RGL.hpp>
#include <RGL/Types.hpp>
#include <RGL/Texture.hpp>

namespace RavEngine{

enum class PostProcessTextureInput : uint8_t{
    Unused,
    EngineColor,
    UserDefined
};

enum class PostProcessOutput : uint8_t{
    EngineColor,
    UserDefined
};

constexpr static uint8_t nPostProcessTextureInputs = 8;
constexpr static uint8_t nPostProcessSamplerInputs = nPostProcessTextureInputs + 1;

struct ScreenEffectConfig{
    std::vector<RGL::PipelineLayoutDescriptor::LayoutBindingDesc> bindings;
    uint8_t pushConstantSize = 0;       // only 128 bytes of total push constant space are allowed
    RGL::BlendFactor
        sourceColorBlendFactor = RGL::BlendFactor::One,
        destinationColorBlendFactor = RGL::BlendFactor::Zero;
};

struct PostProcessInstanceConfig {
    Array<PostProcessTextureInput, nPostProcessTextureInputs> inputconfiguration = { PostProcessTextureInput::EngineColor };
    PostProcessOutput outputConfiguration = PostProcessOutput::EngineColor;
    bool clearOutputBeforeRendering = false;
};

struct BasePushConstantUBO{
    glm::ivec2 dim;
};

struct ScreenEffectInternalConfig {
    const RGL::TextureFormat outputFormat;
};

struct ScreenEffectBase {
    ScreenEffectBase(const std::string_view name, const ScreenEffectConfig& config, const ScreenEffectInternalConfig& internalConfig);

    const auto GetPipeline() const {
        return pipeline;
    }
private:
    RGLRenderPipelinePtr pipeline;
};

struct PostProcessPass : public ScreenEffectBase{
    PostProcessPass(const std::string_view name, const ScreenEffectConfig& config = {});
    

};

struct PostProcessPassInstance{
    PostProcessPassInstance(Ref<PostProcessPass> effect, const PostProcessInstanceConfig& config ) : effect(effect), inputConfiguration(config.inputconfiguration), outputConfiguration(config.outputConfiguration), clearOutputBeforeRendering(config.clearOutputBeforeRendering) {};
    ~PostProcessPassInstance() = default;

    virtual const RGL::untyped_span GetPushConstantData() const{
        return {nullptr,0};     // return nullptr if you do not have additional push constants
    }
    
    Array<RGL::TextureView,nPostProcessTextureInputs> inputBindings;
    Array<RGLSamplerPtr, nPostProcessSamplerInputs> inputSamplerBindings;
    RGL::TextureView outputBinding;
    const auto GetEffect() const{
        return effect;
    }
    using InputConfigurationType = Array<PostProcessTextureInput,nPostProcessTextureInputs>;
    InputConfigurationType inputConfiguration;
    PostProcessOutput outputConfiguration;
    const auto& GetinputConfiguration() const{
        return inputConfiguration;
    }
    virtual dim_t<int> GetUserDefinedOutputSize() const {    // called if using a custom output texture
        return { 0,0 };
    }
    bool clearOutputBeforeRendering = false;
private:
    Ref<PostProcessPass> effect;
};

struct PostProcessEffect{
    Vector<Ref<PostProcessPassInstance>> passes;
    bool enabled = true;
    virtual void Preamble(dim_t<int> targetSize){}
};

struct PostProcessEffectStack{
    Vector<std::unique_ptr<PostProcessEffect>> effects;
    
};

}
#endif
