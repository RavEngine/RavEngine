#if RGL_DX12_AVAILABLE
#include "Types.hpp"
#include "RGLD3D12.hpp"
#include "RGLCommon.hpp"
#include "D3D12RenderPass.hpp"
#include "AftermathIntegration.hpp"
#include <d3d12sdklayers.h>
#include <wrl.h>
#include <format>
#include <iostream>
#include <atlbase.h>
#include <dxcapi.h>

#if __has_include(<pix3.h>)
#include <pix3.h>
#define PIX_ENABLED 1
#endif

#define DX12_USE_AGILITY 1

#define TDR_PIX_CAPTURE 0

// Exports for the Agility SDK. For Windows 10 users, Go here: https://www.nuget.org/packages/Microsoft.Direct3D.D3D12/1.614.0 then unzip it, and place 
// D3D12Core.dll and d3d12SDKLayers.dll in a folder named D3D12 next to the executable.

#if DX12_USE_AGILITY
extern "C" { __declspec(dllexport) extern const UINT D3D12SDKVersion = 614; }
extern "C" { __declspec(dllexport) extern const char* D3D12SDKPath = ".\\D3D12\\"; }
#endif

#define GPU_BASED_VALIDATION 0

#ifdef REFL_ENABLED
#pragma comment(lib, "dxcompiler.lib")	// to get access to new compiler api
#endif

using namespace RGL;
using namespace Microsoft::WRL;

CComPtr<IDxcUtils> dxcUtilsPtr;

PIXBeginEvent_t PIXBeginEvent_fn = nullptr;
PIXEndEvent_t PIXEndEvent_fn = nullptr;

constexpr std::string_view D3D12AutoBreadcrumbOpToString(D3D12_AUTO_BREADCRUMB_OP op)
{
    switch (op)
    {
    case D3D12_AUTO_BREADCRUMB_OP_SETMARKER:
        return "SETMARKER";
    case D3D12_AUTO_BREADCRUMB_OP_BEGINEVENT:
        return "BEGINEVENT";
    case D3D12_AUTO_BREADCRUMB_OP_ENDEVENT:
        return "ENDEVENT";
    case D3D12_AUTO_BREADCRUMB_OP_DRAWINSTANCED:
        return "DRAWINSTANCED";
    case D3D12_AUTO_BREADCRUMB_OP_DRAWINDEXEDINSTANCED:
        return "DRAWINDEXEDINSTANCED";
    case D3D12_AUTO_BREADCRUMB_OP_EXECUTEINDIRECT:
        return "EXECUTEINDIRECT";
    case D3D12_AUTO_BREADCRUMB_OP_DISPATCH:
        return "DISPATCH";
    case D3D12_AUTO_BREADCRUMB_OP_COPYBUFFERREGION:
        return "COPYBUFFERREGION";
    case D3D12_AUTO_BREADCRUMB_OP_COPYTEXTUREREGION:
        return "COPYTEXTUREREGION";
    case D3D12_AUTO_BREADCRUMB_OP_COPYRESOURCE:
        return "COPYRESOURCE";
    case D3D12_AUTO_BREADCRUMB_OP_COPYTILES:
        return "COPYTILES";
    case D3D12_AUTO_BREADCRUMB_OP_RESOLVESUBRESOURCE:
        return "RESOLVESUBRESOURCE";
    case D3D12_AUTO_BREADCRUMB_OP_CLEARRENDERTARGETVIEW:
        return "CLEARRENDERTARGETVIEW";
    case D3D12_AUTO_BREADCRUMB_OP_CLEARUNORDEREDACCESSVIEW:
        return "CLEARUNORDEREDACCESSVIEW";
    case D3D12_AUTO_BREADCRUMB_OP_CLEARDEPTHSTENCILVIEW:
        return "CLEARDEPTHSTENCILVIEW";
    case D3D12_AUTO_BREADCRUMB_OP_RESOURCEBARRIER:
        return "RESOURCEBARRIER";
    case D3D12_AUTO_BREADCRUMB_OP_EXECUTEBUNDLE:
        return "EXECUTEBUNDLE";
    case D3D12_AUTO_BREADCRUMB_OP_PRESENT:
        return "PRESENT";
    case D3D12_AUTO_BREADCRUMB_OP_RESOLVEQUERYDATA:
        return "RESOLVEQUERYDATA";
    case D3D12_AUTO_BREADCRUMB_OP_BEGINSUBMISSION:
        return "BEGINSUBMISSION";
    case D3D12_AUTO_BREADCRUMB_OP_ENDSUBMISSION:
        return "ENDSUBMISSION";
    case D3D12_AUTO_BREADCRUMB_OP_DECODEFRAME:
        return "DECODEFRAME";
    case D3D12_AUTO_BREADCRUMB_OP_PROCESSFRAMES:
        return "PROCESSFRAMES";
    case D3D12_AUTO_BREADCRUMB_OP_ATOMICCOPYBUFFERUINT:
        return "ATOMICCOPYBUFFERUINT";
    case D3D12_AUTO_BREADCRUMB_OP_ATOMICCOPYBUFFERUINT64:
        return "ATOMICCOPYBUFFERUINT64";
    case D3D12_AUTO_BREADCRUMB_OP_RESOLVESUBRESOURCEREGION:
        return "RESOLVESUBRESOURCEREGION";
    case D3D12_AUTO_BREADCRUMB_OP_WRITEBUFFERIMMEDIATE:
        return "WRITEBUFFERIMMEDIATE";
    case D3D12_AUTO_BREADCRUMB_OP_DECODEFRAME1:
        return "DECODEFRAME1";
    case D3D12_AUTO_BREADCRUMB_OP_SETPROTECTEDRESOURCESESSION:
        return "SETPROTECTEDRESOURCESESSION";
    case D3D12_AUTO_BREADCRUMB_OP_DECODEFRAME2:
        return "DECODEFRAME2";
    case D3D12_AUTO_BREADCRUMB_OP_PROCESSFRAMES1:
        return "PROCESSFRAMES1";
    case D3D12_AUTO_BREADCRUMB_OP_BUILDRAYTRACINGACCELERATIONSTRUCTURE:
        return "BUILDRAYTRACINGACCELERATIONSTRUCTURE";
    case D3D12_AUTO_BREADCRUMB_OP_EMITRAYTRACINGACCELERATIONSTRUCTUREPOSTBUILDINFO:
        return "EMITRAYTRACINGACCELERATIONSTRUCTUREPOSTBUILDINFO";
    case D3D12_AUTO_BREADCRUMB_OP_COPYRAYTRACINGACCELERATIONSTRUCTURE:
        return "COPYRAYTRACINGACCELERATIONSTRUCTURE";
    case D3D12_AUTO_BREADCRUMB_OP_DISPATCHRAYS:
        return "DISPATCHRAYS";
    case D3D12_AUTO_BREADCRUMB_OP_INITIALIZEMETACOMMAND:
        return "INITIALIZEMETACOMMAND";
    case D3D12_AUTO_BREADCRUMB_OP_EXECUTEMETACOMMAND:
        return "EXECUTEMETACOMMAND";
    case D3D12_AUTO_BREADCRUMB_OP_ESTIMATEMOTION:
        return "ESTIMATEMOTION";
    case D3D12_AUTO_BREADCRUMB_OP_RESOLVEMOTIONVECTORHEAP:
        return "RESOLVEMOTIONVECTORHEAP";
    case D3D12_AUTO_BREADCRUMB_OP_SETPIPELINESTATE1:
        return "SETPIPELINESTATE1";
    case D3D12_AUTO_BREADCRUMB_OP_INITIALIZEEXTENSIONCOMMAND:
        return "INITIALIZEEXTENSIONCOMMAND";
    case D3D12_AUTO_BREADCRUMB_OP_EXECUTEEXTENSIONCOMMAND:
        return "EXECUTEEXTENSIONCOMMAND";
    case D3D12_AUTO_BREADCRUMB_OP_DISPATCHMESH:
        return "DISPATCHMESH";
    case D3D12_AUTO_BREADCRUMB_OP_ENCODEFRAME:
        return "ENCODEFRAME";
    case D3D12_AUTO_BREADCRUMB_OP_RESOLVEENCODEROUTPUTMETADATA:
        return "RESOLVEENCODEROUTPUTMETADATA";
    default:
        return "Unknown D3D12_AUTO_BREADCRUMB_OP";
    }
}

constexpr std::string_view D3D12_DRED_ALLOCATION_TYPE_to_string(D3D12_DRED_ALLOCATION_TYPE type)
{
    switch (type)
    {
    case D3D12_DRED_ALLOCATION_TYPE_COMMAND_QUEUE:
        return "COMMAND_QUEUE";
    case D3D12_DRED_ALLOCATION_TYPE_COMMAND_ALLOCATOR:
        return "COMMAND_ALLOCATOR";
    case D3D12_DRED_ALLOCATION_TYPE_PIPELINE_STATE:
        return "PIPELINE_STATE";
    case D3D12_DRED_ALLOCATION_TYPE_COMMAND_LIST:
        return "COMMAND_LIST";
    case D3D12_DRED_ALLOCATION_TYPE_FENCE:
        return "FENCE";
    case D3D12_DRED_ALLOCATION_TYPE_DESCRIPTOR_HEAP:
        return "DESCRIPTOR_HEAP";
    case D3D12_DRED_ALLOCATION_TYPE_HEAP:
        return "HEAP";
    case D3D12_DRED_ALLOCATION_TYPE_QUERY_HEAP:
        return "QUERY_HEAP";
    case D3D12_DRED_ALLOCATION_TYPE_COMMAND_SIGNATURE:
        return "COMMAND_SIGNATURE";
    case D3D12_DRED_ALLOCATION_TYPE_PIPELINE_LIBRARY:
        return "PIPELINE_LIBRARY";
    case D3D12_DRED_ALLOCATION_TYPE_VIDEO_DECODER:
        return "VIDEO_DECODER";
    case D3D12_DRED_ALLOCATION_TYPE_VIDEO_PROCESSOR:
        return "VIDEO_PROCESSOR";
    case D3D12_DRED_ALLOCATION_TYPE_RESOURCE:
        return "RESOURCE";
    case D3D12_DRED_ALLOCATION_TYPE_PASS:
        return "PASS";
    case D3D12_DRED_ALLOCATION_TYPE_CRYPTOSESSION:
        return "CRYPTOSESSION";
    case D3D12_DRED_ALLOCATION_TYPE_CRYPTOSESSIONPOLICY:
        return "CRYPTOSESSIONPOLICY";
        return "PROTECTEDRESOURCESESSION";
    case D3D12_DRED_ALLOCATION_TYPE_VIDEO_DECODER_HEAP:
        return "VIDEO_DECODER_HEAP";
    case D3D12_DRED_ALLOCATION_TYPE_COMMAND_POOL:
        return "COMMAND_POOL";
    case D3D12_DRED_ALLOCATION_TYPE_COMMAND_RECORDER:
        return "COMMAND_RECORDER";
    case D3D12_DRED_ALLOCATION_TYPE_STATE_OBJECT:
        return "STATE_OBJECT";
    case D3D12_DRED_ALLOCATION_TYPE_METACOMMAND:
        return "METACOMMAND";
    case D3D12_DRED_ALLOCATION_TYPE_SCHEDULINGGROUP:
        return "SCHEDULINGGROUP";
    case D3D12_DRED_ALLOCATION_TYPE_VIDEO_MOTION_ESTIMATOR:
        return "VIDEO_MOTION_ESTIMATOR";
    case D3D12_DRED_ALLOCATION_TYPE_VIDEO_MOTION_VECTOR_HEAP:
        return "VIDEO_MOTION_VECTOR_HEAP";
    case D3D12_DRED_ALLOCATION_TYPE_VIDEO_EXTENSION_COMMAND:
        return "VIDEO_EXTENSION_COMMAND";
    case D3D12_DRED_ALLOCATION_TYPE_VIDEO_ENCODER:
        return "VIDEO_ENCODER";
    case D3D12_DRED_ALLOCATION_TYPE_VIDEO_ENCODER_HEAP:
        return "VIDEO_ENCODER_HEAP";
    case D3D12_DRED_ALLOCATION_TYPE_INVALID:
        return "INVALID";
    default:
        return "Unknown D3D12_DRED_ALLOCATION_TYPE";
    }
}

void debugLog(const std::string_view str) {
    std::cout << str;
    OutputDebugStringA(str.data());
}

namespace RGL {
    void RGLDeviceRemovedHandler(PVOID context, BOOLEAN)
    {
        ID3D12Device* pDevice = (ID3D12Device*)context;

        auto reason = pDevice->GetDeviceRemovedReason();
        if (reason == S_OK) {
            return; // proper shutdown, no need to go further
        }

#if PIX_ENABLED && TDR_PIX_CAPTURE
        PIXEndCapture(FALSE);
#endif
#if defined(_DEBUG)
       
#if _UWP
        OutputDebugStringW(_com_error(reason, nullptr).ErrorMessage());
#else
        OutputDebugStringA(_com_error(reason, nullptr).ErrorMessage());
#endif

        ComPtr<ID3D12DeviceRemovedExtendedData> pDred;
        DX_CHECK(pDevice->QueryInterface(IID_PPV_ARGS(&pDred)));
        D3D12_DRED_AUTO_BREADCRUMBS_OUTPUT DredAutoBreadcrumbsOutput;
        D3D12_DRED_PAGE_FAULT_OUTPUT DredPageFaultOutput;
        DX_CHECK(pDred->GetAutoBreadcrumbsOutput(&DredAutoBreadcrumbsOutput));
        DX_CHECK(pDred->GetPageFaultAllocationOutput(&DredPageFaultOutput));

        // Custom processing of DRED data can be done here.
        // Log information to console...
        {
            auto node = DredAutoBreadcrumbsOutput.pHeadAutoBreadcrumbNode;
            while (node) {
                debugLog(std::format("DRED Breadcrumb Data:\n\tCommand List:\t{}\n\tCommand Queue:\t{}\n", node->pCommandListDebugNameA == nullptr ? "[Unnamed CommandList]" : node->pCommandListDebugNameA, node->pCommandQueueDebugNameA == nullptr? "[Unnamed CommandQueue]" : node->pCommandQueueDebugNameA));
                for (UINT32 i = 0; i < node->BreadcrumbCount; i++) {
                    auto& command = node->pCommandHistory[i];
                    auto str = D3D12AutoBreadcrumbOpToString(command);
                    const char* failStr = "";
                    if (i == *node->pLastBreadcrumbValue) {
                        failStr = "\t\t <- failed";
                    }
                    debugLog(std::format("\t\t{}{}\n", str, failStr));
                }
                node = node->pNext;
            }
        }

        debugLog(std::format("DRED Page Fault Output:\n\tVirtual Address: {:X}", DredPageFaultOutput.PageFaultVA));
        {
            auto node = DredPageFaultOutput.pHeadExistingAllocationNode;
            while (node) {
                debugLog(std::format("\tDRED Page Fault Existing Allocation Node:\n\t\tObject Name: {}\n\t\tAllocation Type: {}\n",node->ObjectNameA ? node->ObjectNameA : "[Unnamed Object]", D3D12_DRED_ALLOCATION_TYPE_to_string(node->AllocationType)));
                node = node->pNext;
            }
        }
        {
            auto node = DredPageFaultOutput.pHeadRecentFreedAllocationNode;
            while (node) {
                debugLog(std::format("\tDRED Page Fault Recent Freed Allocation Node:\n\t\tObject Name: {}\n\t\tAllocation Type: {}\n", node->ObjectNameA ? node->ObjectNameA : "[Unnamed Object]", D3D12_DRED_ALLOCATION_TYPE_to_string(node->AllocationType)));
                node = node->pNext;
            }
        }
#endif
        FatalError("Device removal triggered!");
    }

    void EnableDebugLayer()
    {
#if defined(_DEBUG)
        // Always enable the debug layer before doing anything DX12 related
        // so all possible errors generated while creating DX12 objects
        // are caught by the debug layer.
        // Enabling the debug layer after creating the ID3D12Device will cause the runtime to remove the device. 
        ComPtr<ID3D12Debug> debugInterface;
        DX_CHECK(D3D12GetDebugInterface(IID_PPV_ARGS(&debugInterface)));
        debugInterface->EnableDebugLayer();

        constexpr static const char* const pixlibname = 
#if _UWP
            "WinPixEventRuntime_UAP.dll";
#else
            "WinPixEventRuntime.dll";
#endif

#if 0
        // load WinPixEventRuntime manually so we don't have to use the NuGet (because it doesn't work with cmake projects)
        mod_WinPixEventRuntime = LoadLibraryA(pixlibname);
        if (mod_WinPixEventRuntime != nullptr) {
            std::cout << std::format("{} found. Capture annotations are enabled.", pixlibname) << std::endl;
            PIXBeginEvent_fn = (decltype(PIXBeginEvent_fn))GetProcAddress(mod_WinPixEventRuntime, "PIXBeginEventOnCommandList\0");
            if (!PIXBeginEvent_fn) {
                auto err = GetLastError();
                FatalError(std::format("Failed to load PIXBeginEventOnCommandList : {}", err));
            }
            PIXEndEvent_fn = (decltype(PIXEndEvent_fn))GetProcAddress(mod_WinPixEventRuntime, "PIXEndEventOnCommandList\0");
            if (!PIXEndEvent_fn) {
                auto err = GetLastError();
                FatalError(std::format("Failed to load PIXEndEventOnCommandList : {}", err));
            }
        }
#endif

        // GPU-based validation
#if GPU_BASED_VALIDATION
        ComPtr<ID3D12Debug1> spDebugController1;
        DX_CHECK(debugInterface->QueryInterface(IID_PPV_ARGS(&spDebugController1)));
        spDebugController1->SetEnableGPUBasedValidation(true);
#endif
        // enable DRED 
        ComPtr<ID3D12DeviceRemovedExtendedDataSettings> pDredSettings;
        DX_CHECK(D3D12GetDebugInterface(IID_PPV_ARGS(&pDredSettings)));

        // Turn on auto-breadcrumbs and page fault reporting.
        pDredSettings->SetAutoBreadcrumbsEnablement(D3D12_DRED_ENABLEMENT_FORCED_ON);
        pDredSettings->SetPageFaultEnablement(D3D12_DRED_ENABLEMENT_FORCED_ON);
#endif
    }

    void RGL::InitD3D12(const RGL::InitOptions& options) {
        Assert(CanInitAPI(RGL::API::Direct3D12), "Direct3D12 cannot be initialized on this platform.");
        RGL::currentAPI = API::Direct3D12;
        EnableDebugLayer();
        InitializeAftermath();
#ifdef REFL_ENABLED
        DxcCreateInstance(CLSID_DxcUtils, IID_PPV_ARGS(&dxcUtilsPtr));
#endif

#if PIX_ENABLED && TDR_PIX_CAPTURE
        auto lib = PIXLoadLatestWinPixGpuCapturerLibrary();
        Assert(lib != nullptr, "Failed to load PIX library");
        PIXCaptureParameters params{
            .GpuCaptureParameters = {
                .FileName = L"TDRCap.pix",
            }
        };
        DX_CHECK(PIXBeginCapture(PIX_CAPTURE_GPU, &params));
#endif
    }

    void RGL::DeintD3D12()
    {
        DeinitAftermath();
    }

    ComPtr<ID3D12DescriptorHeap> CreateDescriptorHeap(ComPtr<ID3D12Device2> device,
        D3D12_DESCRIPTOR_HEAP_TYPE type, uint32_t numDescriptors, D3D12_DESCRIPTOR_HEAP_FLAGS flags)
    {
        ComPtr<ID3D12DescriptorHeap> descriptorHeap;

        D3D12_DESCRIPTOR_HEAP_DESC desc = {};
        desc.NumDescriptors = numDescriptors;
        desc.Type = type;
        desc.Flags = flags;

        DX_CHECK(device->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&descriptorHeap)));

        return descriptorHeap;
    }
    RGLRenderPassPtr CreateRenderPassD3D12(const RenderPassConfig& config)
    {
        return std::make_shared<RenderPassD3D12>(config);
    }

    DXGI_FORMAT rgl2dxgiformat_texture(RGL::TextureFormat format) {
        switch (format) {
        case decltype(format)::BGRA8_Unorm:  return DXGI_FORMAT_R8G8B8A8_UNORM;
        case decltype(format)::RGBA8_Unorm:  return DXGI_FORMAT_R8G8B8A8_UNORM;
        case decltype(format)::RGBA8_Uint:  return DXGI_FORMAT_R8G8B8A8_UINT;
        case decltype(format)::RGBA16_Unorm:  return DXGI_FORMAT_R16G16B16A16_UNORM;
        case decltype(format)::RGBA16_Snorm:  return DXGI_FORMAT_R16G16B16A16_SNORM;
        case decltype(format)::RGBA16_Sfloat:  return DXGI_FORMAT_R16G16B16A16_FLOAT;
        case decltype(format)::RGBA32_Sfloat:  return DXGI_FORMAT_R32G32B32A32_FLOAT;

        case decltype(format)::R8_Uint:  return DXGI_FORMAT_R8_UINT;
        case decltype(format)::R16_Float:  return DXGI_FORMAT_R16_FLOAT;
        case decltype(format)::R32_Uint:  return DXGI_FORMAT_R32_UINT;
        case decltype(format)::R32_Float: return DXGI_FORMAT_R32_FLOAT;

        case decltype(format)::D32SFloat:  return DXGI_FORMAT_D32_FLOAT;
        case decltype(format)::D24UnormS8Uint:  return DXGI_FORMAT_D24_UNORM_S8_UINT;


        case decltype(format)::Undefined:  return DXGI_FORMAT_UNKNOWN;
        default:
            FatalError("Unsupported texture format");
        }
    }

    D3D12_RESOURCE_STATES rgl2d3d12resourcestate(RGL::ResourceLayout layout) {
        switch (layout) {
        case decltype(layout)::Undefined:
        case decltype(layout)::General:
        case decltype(layout)::Reinitialized:
            return D3D12_RESOURCE_STATE_COMMON;

        case decltype(layout)::DepthStencilAttachmentOptimal:
        case decltype(layout)::StencilAttachmentOptimal:
        case decltype(layout)::DepthAttachmentOptimal:
            return D3D12_RESOURCE_STATE_DEPTH_WRITE;

        case decltype(layout)::ColorAttachmentOptimal:
        case decltype(layout)::AttachmentOptimal:
            return D3D12_RESOURCE_STATE_RENDER_TARGET;
        
        case decltype(layout)::DepthReadOnlyOptimal:
        case decltype(layout)::StencilReadOnlyOptimal:
        case decltype(layout)::DepthStencilReadOnlyOptimal:
            return D3D12_RESOURCE_STATE_DEPTH_READ | D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE | D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE;

        case decltype(layout)::ShaderReadOnlyOptimal:
            return D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE | D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE;

        case decltype(layout)::TransferSourceOptimal:
            return D3D12_RESOURCE_STATE_COPY_SOURCE;

        case decltype(layout)::TransferDestinationOptimal:
            return D3D12_RESOURCE_STATE_COPY_DEST;

        case decltype(layout)::DepthReadOnlyStencilAttachmentOptimal:
        case decltype(layout)::DepthAttachmentStencilReadOnlyOptimal:
            return D3D12_RESOURCE_STATE_DEPTH_READ | D3D12_RESOURCE_STATE_RENDER_TARGET;

        case decltype(layout)::ReadOnlyOptimal:
            return D3D12_RESOURCE_STATE_GENERIC_READ;

        case decltype(layout)::Present:
            return D3D12_RESOURCE_STATE_PRESENT;

        default:
            FatalError("layout is not supported");
        }
    }
}
#endif
