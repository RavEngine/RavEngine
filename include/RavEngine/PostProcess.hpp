#pragma once
#include <string_view>
#include <RGL/Pipeline.hpp>
#include "Ref.hpp"
#include "DataStructures.hpp"
#include "mathtypes.hpp"
#include "Texture.hpp"
#include <RGL/Span.hpp>

namespace RGL{
    struct TextureView;
}

namespace RavEngine{

enum class PostProcessTextureInput : uint8_t{
    Unused,
    EngineColor,
    EngineNormal,
    UserDefined
};

enum class PostProcessOutput : uint8_t{
    EngineColor,
    UserDefined
};

constexpr static uint8_t nPostProcessTextureInputs = 8;
constexpr static uint8_t nPostProcessSamplerInputs = nPostProcessTextureInputs + 1;

struct PostProcessConfig{
    std::vector<RGL::PipelineLayoutDescriptor::LayoutBindingDesc> bindings;
    PostProcessOutput output = PostProcessOutput::EngineColor;
    uint8_t pushConstantSize = 0;       // only 128 bytes of total push constant space are allowed
};

struct BasePushConstantUBO{
    glm::ivec2 dim;
};

struct PostProcessPass{
    PostProcessPass(const std::string_view name, const PostProcessConfig& config = {});
    
    const auto GetPipeline() const{
        return pipeline;
    }
private:
    RGLRenderPipelinePtr pipeline;
};

struct PostProcessPassInstance{
    PostProcessPassInstance(Ref<PostProcessPass> effect, const Array<PostProcessTextureInput,nPostProcessTextureInputs>& inputConfiguration = {PostProcessTextureInput::EngineColor});
    ~PostProcessPassInstance();

    virtual const RGL::untyped_span GetPushConstantData() const{
        return {nullptr,0};     // return nullptr if you do not have additional push constants
    }
    using TextureBindingPtrDeleterType =  void(*)(RGL::TextureView*);
    using OutputBindingPtrType = std::unique_ptr<RGL::TextureView,TextureBindingPtrDeleterType> ;
    
    Array<OutputBindingPtrType,nPostProcessTextureInputs> inputBindings;
    Array<RGLSamplerPtr, nPostProcessSamplerInputs> inputSamplerBindings;
    OutputBindingPtrType outputBinding{nullptr, [](RGL::TextureView*){}};  // we use custom deleters to work around a problem with incomplete types and unique_ptr
    const auto GetEffect() const{
        return effect;
    }
    using InputConfigurationType = Array<PostProcessTextureInput,nPostProcessTextureInputs>;
    InputConfigurationType inputConfiguration;
    const auto& GetinputConfiguration() const{
        return inputConfiguration;
    }
private:
    Ref<PostProcessPass> effect;
};

struct PostProcessEffect{
    Vector<Ref<PostProcessPassInstance>> passes;
    bool enabled = true;
    virtual void Preamble(dim_t<int> targetSize){}
};

struct PostProcessEffectStack{
    Vector<Ref<PostProcessEffect>> effects;
    
};

}
