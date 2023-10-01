#pragma once
#include <emscripten/html5_webgpu.h>
#include <RGL/Types.hpp>
#include <RGL/ShaderLibrary.hpp>
#include <string_view>
#include <filesystem>
#include <memory>

namespace RGL{
    struct DeviceWG;

    struct ShaderLibraryWG : public IShaderLibrary{
        const std::shared_ptr<DeviceWG> owningDevice;
        WGPUShaderModule shaderModule;
    
        ShaderLibraryWG(decltype(owningDevice), const std::filesystem::path& pathToShader);
        ShaderLibraryWG(decltype(owningDevice), const std::string_view, const FromSourceConfig& config);
        
        virtual ~ShaderLibraryWG();
    private:
        void InitFromShaderSource(decltype(owningDevice), const std::string_view shadercode);
    };
}
