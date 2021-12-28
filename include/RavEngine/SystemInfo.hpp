#pragma once
#include <cstdint>
#include <string>
#include <bx/bx.h>
#include <bitset>

namespace RavEngine {

    /**
    * This namespace provides functions for getting basic information about the current device.
    * Because most of these functions directly make operating system calls, *DO NOT CALL THESE EVERY FRAME!*
    * Doing so will cause severe performance degradation. Instead, invoke the functions once and cache, or use a 
    * Timed system with an interval no faster than 1 second. 
    */
	namespace SystemInfo {
		/**
		@return the number of logical cores on the device
		*/
		uint16_t NumLogicalProcessors();
    
        /**
         @return the string name of the GPU, as reported by the operating system (eg "Intel(R) Core(TM) i9-9880H CPU @ 2.30GHz")
         */
        std::string CPUBrandString();
	
		std::string GPUBrandString();
    
        constexpr static bool IsMobile(){
            return BX_PLATFORM_IOS || BX_PLATFORM_RPI || BX_PLATFORM_OS_MOBILE;
        }
    
        enum class EArchitecture{
            Unknown, x86_64, aarch64, aarch64e
        };
    
        constexpr static EArchitecture Architecture(){
            #if defined __aarch64__ || defined _M_ARM       // first is clang/gcc, second is msvc
                return EArchitecture::aarch64;
            #elif defined __x86_64__ || defined _M_X64      // first is clang/gcc, second is msvc
                return EArchitecture::x86_64;
            #else
                return EArchitecture::Unknown;
            #endif
        }
    
    /**
     @return Printable version of the architecture string
     */
        constexpr static const char* ArchitectureString(){
            
            switch(Architecture()){
                case EArchitecture::Unknown:
                    return "Unknown";
                case EArchitecture::x86_64:
                    return "x86_64";
                case EArchitecture::aarch64:
                    return "aarch64";
                case EArchitecture::aarch64e:
                    return "aarch64e";
                default:
                    return "Error";
            }
            
        };
    
        /**
         @return the current operating system name
         */
        std::string OperatingSystemNameString();
    
        /**
        @return total system memroy in MB
         */
        uint32_t SystemRAM();
    
        struct OSVersion{
            uint16_t major = 0, minor = 0, patch = 0, extra = 0;
        };
        OSVersion OperatingSystemVersion();
    
        uint32_t GPUVRAM();
        
        uint32_t GPUVRAMinUse();
    
        struct GPUFeatures{
            enum Features{
                iDrawIndirect,
                iHDR10,
                iHIDPI,
                iOcclusionQuery,
                iDMA,
                iReadback,
                iUint10Attribute,
                iHalfAttribute,
                Count
            };
            std::bitset<Features::Count> features{0};
            bool DrawIndirect(){
                return features[Features::iDrawIndirect];
            }
            bool HDRI10(){
                return features[Features::iHDR10];
            }
            bool OcclusionQuery(){
                return features[Features::iOcclusionQuery];
            }
            bool DMA(){
                return features[Features::iDMA];
            }
            bool Readback(){
                return features[Features::iReadback];
            }
            bool Uint10Attribute(){
                return features[Features::iUint10Attribute];
            }
            bool HalfAttribute(){
                return features[Features::iHalfAttribute];
            }
        };
        GPUFeatures GetSupportedGPUFeatures();

        struct PCIDevice {
            uint16_t vendorID;
            uint16_t deviceID;
        };
        PCIDevice GPUPCIData();
	}
}
