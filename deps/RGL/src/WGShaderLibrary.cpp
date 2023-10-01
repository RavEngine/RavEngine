#if RGL_WEBGPU_AVAILABLE
#include "WGShaderLibrary.hpp"
#include <librglc.hpp>
#include <streambuf>
#include <fstream>
#include "WGDevice.hpp"
#include "RGLCommon.hpp"

namespace RGL{
    ShaderLibraryWG::ShaderLibraryWG(decltype(owningDevice) owningDevice, const std::filesystem::path& pathToShader) : owningDevice(owningDevice){
        std::ifstream t(pathToShader);
        if (!t){
            FatalError("Could not open file");
        }
        std::string str((std::istreambuf_iterator<char>(t)),
                 std::istreambuf_iterator<char>());

        InitFromShaderSource(owningDevice, str);
    }
    ShaderLibraryWG::ShaderLibraryWG(decltype(owningDevice) owningDevice, const std::string_view, const FromSourceConfig& config) : owningDevice(owningDevice){
#ifdef RGL_CAN_RUNTIME_COMPILE  // defined in CMake
    auto result = librglc::CompileString(source, librglc::API::WebGPU, static_cast<librglc::ShaderStage>(config.stage), {
        .entrypointOutputName = "transient_fn"
    });
    InitFromShaderSource(owningDevice, result);
#endif
    }
    ShaderLibraryWG::~ShaderLibraryWG(){
        wgpuShaderModuleRelease(shaderModule);
    }
    void ShaderLibraryWG::InitFromShaderSource(decltype(owningDevice) owningDevice, const std::string_view shadercode){
        WGPUShaderModuleWGSLDescriptor shaderCodeDesc;
        // Set the chained struct's header
        shaderCodeDesc.chain.next = nullptr;
        shaderCodeDesc.chain.sType = WGPUSType_ShaderModuleWGSLDescriptor;
        shaderCodeDesc.code = shadercode.data();
        WGPUShaderModuleDescriptor moduleDesc;
        moduleDesc.nextInChain = &shaderCodeDesc.chain;
        shaderModule = wgpuDeviceCreateShaderModule(owningDevice->device, &moduleDesc);
    }
}

#endif