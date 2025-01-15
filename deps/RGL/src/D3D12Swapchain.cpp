#if RGL_DX12_AVAILABLE
#include "D3D12Swapchain.hpp"
#include "D3D12Surface.hpp"
#include <dxgi1_5.h>
#include <directx/d3d12.h>
#include "D3D12Device.hpp"
#include "D3D12CommandQueue.hpp"
#include <directx/d3dx12.h>
#include "D3D12Texture.hpp"

#undef min
#undef max

using namespace Microsoft::WRL;


namespace RGL {
    // gsync, freesync, etc
    bool CheckTearingSupport()
    {
        BOOL allowTearing = FALSE;

        // Rather than create the DXGI 1.5 factory interface directly, we create the
        // DXGI 1.4 interface and query for the 1.5 interface. This is to enable the 
        // graphics debugging tools which will not support the 1.5 factory interface 
        // until a future update.
        ComPtr<IDXGIFactory4> factory4;
        if (SUCCEEDED(CreateDXGIFactory1(IID_PPV_ARGS(&factory4))))
        {
            ComPtr<IDXGIFactory5> factory5;
            if (SUCCEEDED(factory4.As(&factory5)))
            {
                if (FAILED(factory5->CheckFeatureSupport(
                    DXGI_FEATURE_PRESENT_ALLOW_TEARING,
                    &allowTearing, sizeof(allowTearing))))
                {
                    allowTearing = FALSE;
                }
            }
        }

        return allowTearing == TRUE;
    }

    ComPtr<IDXGISwapChain4> CreateSwapChain(const void* hWndPtr,
        ComPtr<ID3D12CommandQueue> commandQueue,
        uint32_t width, uint32_t height, uint32_t bufferCount)
    {
        ComPtr<IDXGISwapChain4> dxgiSwapChain4;
        ComPtr<IDXGIFactory4> dxgiFactory4;
        UINT createFactoryFlags = 0;
#if defined(_DEBUG)
        createFactoryFlags = DXGI_CREATE_FACTORY_DEBUG;
#endif

        DX_CHECK(CreateDXGIFactory2(createFactoryFlags, IID_PPV_ARGS(&dxgiFactory4)));

        // now describe how we want it to work
        DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
        swapChainDesc.Width = width;
        swapChainDesc.Height = height;
        swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        swapChainDesc.Stereo = FALSE;
        swapChainDesc.SampleDesc = { 1, 0 };
        swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        swapChainDesc.BufferCount = bufferCount;
        swapChainDesc.Scaling = DXGI_SCALING_STRETCH;               // what hapens when the backbuffer is not the size of the target
        swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;   // for going as fast as possible (no vsync), discard frames in-flight to reduce latency
        swapChainDesc.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;
        // It is recommended to always allow tearing if tearing support is available.
        swapChainDesc.Flags = CheckTearingSupport() ? DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING : 0;

        ComPtr<IDXGISwapChain1> swapChain1;

        auto hwnd = *static_cast<const HWND*>(hWndPtr);
        DX_CHECK(dxgiFactory4->CreateSwapChainForHwnd(
            commandQueue.Get(),
            hwnd,
            &swapChainDesc,
            nullptr,
            nullptr,
            &swapChain1));

        // Disable the Alt+Enter fullscreen toggle feature. Switching to fullscreen
       // will be handled manually.
        DX_CHECK(dxgiFactory4->MakeWindowAssociation(hwnd, DXGI_MWA_NO_ALT_ENTER));

        DX_CHECK(swapChain1.As(&dxgiSwapChain4));

        return dxgiSwapChain4;
    }

    SwapchainD3D12::SwapchainD3D12(decltype(owningDevice) device, std::shared_ptr<SurfaceD3D12> surface, int width, int height, std::shared_ptr<CommandQueueD3D12> presentQueue) : owningDevice(device)
    {
        backbufferTextures.reserve(g_NumFrames);
        swapchain = CreateSwapChain(surface->windowHandle, presentQueue->GetD3D12CommandQueue(), width, height, g_NumFrames);
        device->internalQueue->Flush();
        UpdateRenderTargetViews(owningDevice->device, swapchain, owningDevice->RTVHeap.value());
        tearingSupported = CheckTearingSupport();
    }

    void SwapchainD3D12::UpdateRenderTargetViews(ComPtr<ID3D12Device2> device, ComPtr<IDXGISwapChain4> swapChain, D3D12DynamicDescriptorHeap<2048>& descriptorHeap)
    {
        auto rtvDescriptorSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

        // deallocate resources
        DXGI_SWAP_CHAIN_DESC1 desc;
        swapChain->GetDesc1(&desc);
        backbufferTextures.clear();
        // note: we do not need to manually release the descriptor indicies, because 
        // the framebuffers are wrapped in Texture handles and 
        // automatically release their own IDs when they are destroyed

        for (int i = 0; i < g_NumFrames; ++i)
        {

            auto idx = rtvIndices[i] = descriptorHeap.AllocateSingle();

            ComPtr<ID3D12Resource> backBuffer;
            DX_CHECK(swapChain->GetBuffer(i, IID_PPV_ARGS(&backBuffer)));
            backBuffer->SetName(L"Swapchain Buffer");

            auto handle = descriptorHeap.GetCpuHandle(idx);

            device->CreateRenderTargetView(backBuffer.Get(), nullptr, handle);

            backbuffers[i] = backBuffer;
            backbufferTextures.emplace_back(backBuffer, Dimension{ desc.Width,desc.Height }, idx, owningDevice);
        }
        initialized = true;
    }

    void SwapchainD3D12::Resize(uint32_t width, uint32_t height)
	{
        // Don't allow 0 size swap chain back buffers.
        width = std::max(1u, width);
        height = std::max(1u, height);

        // Flush the GPU queue to make sure the swap chain's back buffers
        // are not being referenced by an in-flight command list.

        owningDevice->Flush();
        backbufferTextures.clear(); // need to release all existing references to the textures before can resize

        auto currentidx = swapchain->GetCurrentBackBufferIndex();

        for (int i = 0; i < g_NumFrames; ++i)
        {
            // Any references to the back buffers must be released
            // before the swap chain can be resized.
            backbuffers[i].Reset();            
        }
        DXGI_SWAP_CHAIN_DESC swapChainDesc = {};
        DX_CHECK(swapchain->GetDesc(&swapChainDesc));
        DX_CHECK(swapchain->ResizeBuffers(g_NumFrames, width, height,
            swapChainDesc.BufferDesc.Format, swapChainDesc.Flags));

        UpdateRenderTargetViews(owningDevice->device, swapchain, owningDevice->RTVHeap.value());
        for (auto& buffer : backbufferTextures) {
            buffer.size = Dimension{ width,height };
        }
	}
	void SwapchainD3D12::GetNextImage(uint32_t* index)
	{
        *index = swapchain->GetCurrentBackBufferIndex();
	}
	ITexture* SwapchainD3D12::ImageAtIndex(uint32_t index)
	{
        return &backbufferTextures[index];
	}
	void SwapchainD3D12::Present(const SwapchainPresentConfig& config)
	{
        UINT syncInterval = vsync ? 1 : 0;
        UINT presentFlags = (tearingSupported && !vsync) ? DXGI_PRESENT_ALLOW_TEARING : 0;
        swapchain->Present(syncInterval, presentFlags);
	}
    void SwapchainD3D12::SetVsyncMode(bool mode)
    {
        vsync = mode;
    }
    SwapchainD3D12::~SwapchainD3D12()
    {
    }
}

#endif
