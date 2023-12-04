#pragma once
#include <string_view>
#include <RGL/Pipeline.hpp>
#include "Ref.hpp"
#include "DataStructures.hpp"
#include "mathtypes.hpp"

namespace RavEngine{

struct PostProcessConfig{
    std::vector<RGL::PipelineLayoutDescriptor::LayoutBindingDesc> bindings;
    uint8_t pushConstantSize = 0;       // only 128 bytes of total push constant space are allowed
};

struct BasePushConstantUBO{
    glm::vec2 dim;
};

struct PostProcessEffect{
    PostProcessEffect(const std::string_view name, const PostProcessConfig& config = {});
private:
    RGLRenderPipelinePtr pipeline;
};

struct PostProcessEffectInstance{
    PostProcessEffectInstance(Ref<PostProcessEffect> effect) : effect(effect){}
    
    virtual const std::byte* GetPushConstantData() const{
        return nullptr;     // return nullptr if you do not have additional push constants
    }
private:
    Ref<PostProcessEffect> effect;
    
};

struct PostProcessEffectStack{
    Vector<Ref<PostProcessEffectInstance>> effects;
};

}
