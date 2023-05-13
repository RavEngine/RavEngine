
#include "Material.hpp"
#include <sstream>
#include <fstream>
#include <RenderEngine.hpp>
#include "mathtypes.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "Filesystem.hpp"
#include "Common3D.hpp"
#include "App.hpp"
#include <typeindex>
#include "Debug.hpp"
#include "VirtualFileSystem.hpp"
#include <RGL/RGL.hpp>
#include <RGL/Device.hpp>
#include <RGL/Pipeline.hpp>

using namespace std;
using namespace RavEngine;

/**
Create a material given a shader. Also registers it in the material manager
@param shader the path to the shader
*/
Material::Material(const std::string& name, const MaterialConfig& config) : name(name) {

	auto device = GetApp()->GetRenderEngine().GetDevice();


	//get all shader files for this programs
	auto vertshaderName = StrFormat("{}.vsh", name);
	auto fragShaderName = StrFormat("{}.fsh", name);

    vertShader = LoadShaderByFilename(vertshaderName, device);
    fragShader = LoadShaderByFilename(fragShaderName, device);

    auto samplerPtr = GetApp()->GetRenderEngine().textureSampler;

    pipelineLayout = device->CreatePipelineLayout({
        .bindings = config.bindings,
        .boundSamplers = {
            samplerPtr,
            samplerPtr,
            samplerPtr,
            samplerPtr,
            samplerPtr,
            samplerPtr,
            samplerPtr,
            samplerPtr
        },
        .constants = {
            {
               sizeof(RenderEngine::DeferredUBO) + config.pushConstantSize, 0, RGL::StageVisibility(RGL::StageVisibility::Vertex | RGL::StageVisibility::Fragment)
            }
        },
    });

    RGL::RenderPipelineDescriptor rpd{
        .stages = {
            {
                .type = RGL::ShaderStageDesc::Type::Vertex,
                .shaderModule = vertShader,
            },
            {
                .type = RGL::ShaderStageDesc::Type::Fragment,
                .shaderModule = fragShader,
            }
        },
        .vertexConfig = config.vertConfig,
        .inputAssembly = {
            .topology = RGL::PrimitiveTopology::TriangleList,
        },
        .rasterizerConfig = {
            .windingOrder = RGL::WindingOrder::Counterclockwise,
        },
        .colorBlendConfig = config.colorBlendConfig,
        .depthStencilConfig = {
            .depthFormat = RGL::TextureFormat::D32SFloat,
            .depthTestEnabled = config.depthTestEnabled,
            .depthWriteEnabled = config.depthWriteEnabled,
            .depthFunction = config.depthCompareFunction,
        },
        .pipelineLayout = pipelineLayout,
    };

    renderPipeline = device->CreateRenderPipeline(rpd);
}

Material::~Material() {
    // enqueue the pipeline and layout for deletion
    auto& renderer = GetApp()->GetRenderEngine();
    renderer.gcPipelineLayout.enqueue(pipelineLayout);
    renderer.gcRenderPipeline.enqueue(renderPipeline);
}
