#include <RGL/Core.hpp>

#include "RGLCommon.hpp"
#include "Device.hpp"
#include "RenderPass.hpp"

#if RGL_DX12_AVAILABLE
#include "D3D12Device.hpp"
#include "RGLD3D12.hpp"
#endif

#if RGL_VK_AVAILABLE
#include "RGLVk.hpp"
#include "VkDevice.hpp"
#endif

#if RGL_MTL_AVAILABLE
#include "RGLMTL.hpp"
#include "MTLDevice.hpp"
#endif

#include <iostream>

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <format>
#endif

using namespace RGL;

STATIC(RGL::currentAPI) = API::Uninitialized;

namespace RGL {

    static callback_t callbackFn = [](MessageSeverity severity, const std::string& message) {
        constexpr auto severityToStr = [](MessageSeverity severity) {
            switch (severity) {
            case MessageSeverity::Info:
                return "Info";
            case MessageSeverity::Warning:
                return "Warning";
            case MessageSeverity::Error:
                return "Error";
            case MessageSeverity::Fatal:
                return "Fatal";
            }
        };

        std::cout << "RGL [" << severityToStr(severity) << "] - " << message << "\n";
#if _WIN32
        std::string str = std::format("RGL [{}] - {}\n", severityToStr(severity), message);
        OutputDebugStringA(str.c_str());
#endif
    };

    RGLDevicePtr RGL::IDevice::CreateSystemDefaultDevice()
    {
        switch (RGL::currentAPI) {
        case API::Uninitialized:
            FatalError("RGL is not initialized! Call RGL::Init before using any RGL functions.");
            break;
#if RGL_DX12_AVAILABLE
        case API::Direct3D12:
            return CreateDefaultDeviceD3D12();
#endif
#if RGL_VK_AVAILABLE
        case API::Vulkan:
            return CreateDefaultDeviceVk();
#endif
#if RGL_MTL_AVAILABLE
        case API::Metal:
            return CreateDefaultDeviceMTL();
#endif
        default:
            FatalError("Invalid API");

        }
        return RGLDevicePtr();
    }


    void Init(const InitOptions& options)
    {
        if (options.callback) {
            callbackFn = options.callback;
        }

        if (options.api == API::PlatformDefault) {
            // need to figure out what API to use
#if defined(_WIN32) && RGL_DX12_AVAILABLE
            InitD3D12(options);
#elif __APPLE__
            InitMTL(options);
#elif __linux__ || RGL_VK_AVAILABLE
            InitVk(options);
#else

#endif
        }
        else {
            switch (options.api) {
#if RGL_MTL_AVAILABLE
            case API::Metal:
                InitMTL(options);
                break;
#endif
#if RGL_VK_AVAILABLE
            case API::Vulkan:
                InitVk(options);
                break;
#endif
#if RGL_DX12_AVAILABLE
            case API::Direct3D12:
                InitD3D12(options);
                break;
#endif
            case API::Noop:
                break;
            default:
                FatalError("Cannot load invalid API");
                break;
            }
        }
    }

    void Shutdown() {
        switch (RGL::currentAPI) {
#if RGL_DX12_AVAILABLE
        case API::Direct3D12:
            DeintD3D12();
            break;
#endif
#if RGL_VK_AVAILABLE
        case API::Vulkan:
            DeinitVk();
            break;
#endif
#if RGL_MTL_AVAILABLE
        case API::Metal:
            DeinitMTL();
            break;
#endif
        default:
            FatalError("not implemented for this API");
        }

        RGL::currentAPI = API::Uninitialized;
    }

    bool CanInitAPI(API api)
    {
        return std::find(std::begin(APIsAvailable), std::end(APIsAvailable), api) != std::end(APIsAvailable);
    }

    API CurrentAPI() {
        return RGL::currentAPI;
    }

    const char* APIToString(API api) {
        switch (api) {
        case API::Metal:
            return "Metal";
        case API::Direct3D12:
            return "Direct3D12";
        case API::Vulkan:
            return "Vulkan";
        case API::Noop:
            return "Noop";
        default:
            FatalError("APIToString: invalid API");
            return "";
        }
    }

    void LogMessage(MessageSeverity severity, const std::string& str) {
        callbackFn(severity, str);

        // crash the program 
        // the user should not try to catch this, UB lies beyond
        if (severity == MessageSeverity::Fatal) {
            throw std::runtime_error(str);
        }
    }
    void FatalError(const std::string& str) {
        LogMessage(MessageSeverity::Fatal, str);
    }

    void FatalError(const std::wstring& wstr) {
        std::string result;
        result.resize(wstr.size());
#if _UWP
        wcstombs_s(nullptr, result.data(), result.size(), wstr.data(), _TRUNCATE);
#else
        std::wcstombs(result.data(), wstr.data(), result.size());
#endif


        LogMessage(MessageSeverity::Fatal, result);
    }

    RGLRenderPassPtr CreateRenderPass(const RenderPassConfig& config) {
        switch (RGL::currentAPI) {
#if RGL_DX12_AVAILABLE
        case API::Direct3D12:
            return CreateRenderPassD3D12(config);
            break;
#endif
#if RGL_VK_AVAILABLE
        case API::Vulkan:
            return CreateRenderPassVk(config);
            break;
#endif
#if RGL_MTL_AVAILABLE
        case API::Metal:
                return CreateRenderPassMTL(config);
            break;
#endif
        default:
            FatalError("not implemented for this API");
        }
    }
}
