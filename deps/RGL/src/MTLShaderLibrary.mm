#if RGL_MTL_AVAILABLE
#import <Metal/Metal.h>
#include "MTLShaderLibrary.hpp"
#include "MTLDevice.hpp"
#include "RGLMTL.hpp"
#include <librglc.hpp>

namespace RGL{
ShaderLibraryMTL::ShaderLibraryMTL(decltype(owningDevice) owningDevice, const std::string_view fnstr_view) : owningDevice(owningDevice){
    NSString* fnstr = [NSString stringWithUTF8String:fnstr_view.data()];
    function = [owningDevice->defaultLibrary newFunctionWithName:fnstr];
}
ShaderLibraryMTL::ShaderLibraryMTL(decltype(owningDevice) owningDevice, const std::string_view source, const FromSourceConfig& config) : owningDevice(owningDevice){
    auto result = librglc::CompileString(source, librglc::API::Metal, static_cast<librglc::ShaderStage>(config.stage), {
        .entrypointOutputName = "transient_fn"
    });
    
    MTLCompileOptions* options = [MTLCompileOptions new];
    
    NSError* err;
    library = [owningDevice->device newLibraryWithSource:[NSString stringWithUTF8String:result.data()] options:options error:&err];
        
    if (err){
        auto errstr = [err localizedFailureReason];
        throw std::runtime_error([errstr UTF8String]);
    }
    
    function = [library newFunctionWithName:@"transient_fn"];

}
}
#endif
