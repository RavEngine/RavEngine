#include "Utilities.hpp"
#include <RGL/RGL.hpp>
#include <RGL/Device.hpp>
#include "App.hpp"
#include "RenderEngine.hpp"

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


RGLShaderLibraryPtr RavEngine::LoadShaderByFilename(const std::string& name, RGLDevicePtr device)
{
    auto shaderRelativePath = name;// GetShader(name);

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
    std::filesystem::path shaderPath;
    if (RGL::CurrentAPI() == RGL::API::Vulkan) {
        shaderPath = std::filesystem::path("Vulkan") / (shaderRelativePath + ".spv");
    }
    else {
        shaderPath = (shaderRelativePath + ".cso");
    }

    return device->CreateShaderLibraryFromPath(shaderPath);
#endif

}
