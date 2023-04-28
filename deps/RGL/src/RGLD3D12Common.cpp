#if RGL_DX12_AVAILABLE
#include "Types.hpp"
#include "RGLD3D12.hpp"
#include "RGLCommon.hpp"
#include "D3D12RenderPass.hpp"
#include <d3d12sdklayers.h>
#include <wrl.h>

using namespace RGL;
using namespace Microsoft::WRL;

namespace RGL {
    void RGLDeviceRemovedHandler(PVOID context, BOOLEAN)
    {
#if defined(_DEBUG)
        ID3D12Device* pDevice = (ID3D12Device*)context;

        auto reason = pDevice->GetDeviceRemovedReason();
        if (reason == S_OK) {
            return; // proper shutdown, no need to go further
        }
        OutputDebugStringA(_com_error(reason, nullptr).ErrorMessage());

        ComPtr<ID3D12DeviceRemovedExtendedData> pDred;
        DX_CHECK(pDevice->QueryInterface(IID_PPV_ARGS(&pDred)));
        D3D12_DRED_AUTO_BREADCRUMBS_OUTPUT DredAutoBreadcrumbsOutput;
        D3D12_DRED_PAGE_FAULT_OUTPUT DredPageFaultOutput;
        DX_CHECK(pDred->GetAutoBreadcrumbsOutput(&DredAutoBreadcrumbsOutput));
        DX_CHECK(pDred->GetPageFaultAllocationOutput(&DredPageFaultOutput));
        // Custom processing of DRED data can be done here.
        // Produce telemetry...
        // Log information to console...
        // break into a debugger...
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
    }

    void RGL::DeintD3D12()
    {
        // as of now, do nothing
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

        case decltype(format)::R32_Uint:  return DXGI_FORMAT_R32_UINT;

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

        case decltype(layout)::ColorAttachmentOptimal:
        case decltype(layout)::DepthStencilAttachmentOptimal:
        case decltype(layout)::DepthAttachmentOptimal:
        case decltype(layout)::StencilAttachmentOptimal:
        case decltype(layout)::AttachmentOptimal:
            return D3D12_RESOURCE_STATE_RENDER_TARGET;

        case decltype(layout)::DepthStencilReadOnlyOptimal:
            return D3D12_RESOURCE_STATE_DEPTH_READ;

        case decltype(layout)::ShaderReadOnlyOptimal:
            return D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE | D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE;

        case decltype(layout)::TransferSourceOptimal:
            return D3D12_RESOURCE_STATE_COPY_SOURCE;

        case decltype(layout)::TransferDestinationOptimal:
            return D3D12_RESOURCE_STATE_COPY_DEST;

        case decltype(layout)::DepthReadOnlyStencilAttachmentOptimal:
        case decltype(layout)::DepthAttachmentStencilReadOnlyOptimal:
            return D3D12_RESOURCE_STATE_DEPTH_READ | D3D12_RESOURCE_STATE_RENDER_TARGET;

        case decltype(layout)::DepthReadOnlyOptimal:
        case decltype(layout)::StencilReadOnlyOptimal:
            return D3D12_RESOURCE_STATE_DEPTH_READ;

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
