#include "Utilities.hpp"
#if !RVE_SERVER
#include <RGL/RGL.hpp>
#include <RGL/Device.hpp>
#include "RenderEngine.hpp"
#endif
#include "App.hpp"
#include "VirtualFileSystem.hpp"

using namespace RavEngine;
#if !RVE_SERVER
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
        return VFormat("shaders/{}/{}", backendPath, (name + extension));

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
    auto name_copy = name;
    std::replace(name_copy.begin(),name_copy.end(),'.','_');
    return device->CreateShaderLibraryFromName(name_copy);
#else

#if 1
    const char* ext = "";
    if (RGL::CurrentAPI() == RGL::API::Vulkan) {
        ext = ".spv";
    }
    else if (RGL::CurrentAPI() == RGL::API::Direct3D12) {
        ext = ".cso";
    }

    auto path = VFormat("{}/{}{}", RGL::APIToString(RGL::CurrentAPI()), name, ext);
    auto shaderBytes = resources.GetShaderData(path);
    //auto shaderBytes = resources.FileContentsAt<std::vector<uint8_t>>(path.c_str(),false);
    return device->CreateShaderLibraryFromBytes({ reinterpret_cast<const uint8_t*>(shaderBytes.data()), shaderBytes.size()});
#else
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
#endif
    Debug::Fatal("Failed to load shader: engine is not compiled correctly");
    return nullptr;
}
#endif
