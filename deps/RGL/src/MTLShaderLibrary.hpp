#pragma once
#include <RGL/Types.hpp>
#include <RGL/ShaderLibrary.hpp>
#include "MTLObjCCompatLayer.hpp"
#include <string_view>
#include <memory>

namespace RGL{
    struct DeviceMTL;

    struct ShaderLibraryMTL : public IShaderLibrary{
        OBJC_ID(MTLFunction) function = nullptr;
        OBJC_ID(MTLLibrary) library = nullptr;
        const std::shared_ptr<DeviceMTL> owningDevice;
    
        
        ShaderLibraryMTL(decltype(owningDevice), const std::string_view);
        ShaderLibraryMTL(decltype(owningDevice), const std::string_view, const FromSourceConfig& config);
        
        virtual ~ShaderLibraryMTL(){}
    };
}
