#pragma once
#include <cstdint>
#include <string>

namespace RavEngine {
	namespace SystemInfo {
		/**
		@return the number of logical cores on the device
		*/
		uint16_t NumLogicalProcessors();
    
        /**
         @return the string name of the GPU, as reported by the operating system (eg "Intel(R) Core(TM) i9-9880H CPU @ 2.30GHz")
         */
        std::string CPUBrandString();
    
        constexpr static bool IsMobile(){
            return false;
        }
    
        enum class EArchitecture{
            Unknown, x86_64, aarch64, aarch64e
        };
    
    constexpr static EArchitecture Architecture(){
        #if defined __aarch64__ || defined _M_ARM    // first is clang/gcc, second is msvc
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

		std::string GPUBrandString();
	}
}
