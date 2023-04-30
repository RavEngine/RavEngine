
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


auto GetShader(const std::string& name) {
    auto getShaderPathname = [](const std::string& name) {
        const char* backendPath;
        const char* extension;
        switch (RGL::CurrentAPI()) {
        case RGL::API::Vulkan:
            backendPath = "Vulkan";
            extension = ".spv";
            break;
        case RGL::API::Direct3D12:
            backendPath = "Direct3D12";
            extension = ".cso";
            break;
        default:
            throw std::runtime_error("Shader loading not implemented");
        }
        return StrFormat("shaders/{}/{}", backendPath, (name + extension));

    };
#if __APPLE__
    // metal uses the Xcode-created metallib in resources, which have a different naming convention
    auto tempstr = name;
    std::replace(tempstr.begin(), tempstr.end(), '.', '_');
	return tempstr;
#else
    return getShaderPathname(name);
#endif
}

/**
Create a material given a shader. Also registers it in the material manager
@param shader the path to the shader
*/
Material::Material(const std::string& name, const MaterialConfig& config) : name(name) {

	auto device = GetApp()->GetRenderEngine().GetDevice();


	//get all shader files for this programs
	auto vertshaderName = StrFormat("{}.vsh", name);
	auto fragShaderName = StrFormat("{}.fsh", name);

    auto vertShaderPath = GetShader(vertshaderName);
    auto fragShaderPath = GetShader(fragShaderName);

    auto& resources = GetApp()->GetResources();
#if __APPLE__
    device->CreateShaderLibraryFromName(vertShaderPath);
    device->CreateShaderLibraryFromName(fragShaderPath);
#else
#if 0
    auto vertex_src = resources.FileContentsAt<std::vector<uint8_t>>(vertShaderPath.c_str());
    auto fragment_src = resources.FileContentsAt<std::vector<uint8_t>>(fragShaderPath.c_str());



    vertShader = device->CreateShaderLibraryFromBytes(vertex_src);
    fragShader = device->CreateShaderLibraryFromBytes(fragment_src);
#endif
    //TODO: this is temporary code for shader loading
    std::filesystem::path vertPath, fragPath;
    if (RGL::CurrentAPI() == RGL::API::Vulkan) {
        vertPath = std::filesystem::path("Vulkan") / (vertshaderName + ".spv");
        fragPath = std::filesystem::path("Vulkan") / (fragShaderName + ".spv");
    }
    else {
        vertPath = (vertshaderName + ".cso");
        fragPath = (fragShaderName + ".cso");
    }

    vertShader = device->CreateShaderLibraryFromPath(vertPath);
    fragShader = device->CreateShaderLibraryFromPath(fragPath);
#endif

    pipelineLayout = device->CreatePipelineLayout({
        .constants = {
            {
                sizeof(RenderEngine::DeferredUBO), 0, RGL::StageVisibility(RGL::StageVisibility::Vertex | RGL::StageVisibility::Fragment)
            }
        }
    });

    constexpr static uint32_t width = 640, height = 480;

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
        .viewport = {
            .width = width,
            .height = height
        },
        .scissor = {
            .extent = {width, height}
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
