#pragma once
#include <string_view>
#include <RGL/Pipeline.hpp>
#include "Ref.hpp"
#include "DataStructures.hpp"
#include "mathtypes.hpp"
#include "Texture.hpp"
#include <RGL/Span.hpp>

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

struct PostProcessConfig{
    std::vector<RGL::PipelineLayoutDescriptor::LayoutBindingDesc> bindings;
    Array<PostProcessTextureInput,nPostProcessTextureInputs> inputs{PostProcessTextureInput::EngineColor};
    PostProcessOutput output = PostProcessOutput::EngineColor;
    uint8_t pushConstantSize = 0;       // only 128 bytes of total push constant space are allowed
};

struct BasePushConstantUBO{
    glm::ivec2 dim;
};

struct PostProcessPass{
    PostProcessPass(const std::string_view name, const PostProcessConfig& config = {});
    
    const auto& GetinputConfiguration() const{
        return inputConfiguration;
    }
    const auto GetPipeline() const{
        return pipeline;
    }
private:
    Array<PostProcessTextureInput,nPostProcessTextureInputs> inputConfiguration;
    RGLRenderPipelinePtr pipeline;
};

struct PostProcessPassInstance{
    PostProcessPassInstance(Ref<PostProcessPass> effect) : effect(effect){}
    
    virtual const RGL::untyped_span GetPushConstantData() const{
        return {nullptr,0};     // return nullptr if you do not have additional push constants
    }
    Array<Ref<Texture>,nPostProcessTextureInputs> inputBindings;
    Ref<Texture> outputBinding;
    const auto GetEffect() const{
        return effect;
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
