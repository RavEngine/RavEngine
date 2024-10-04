#if RGL_DX12_AVAILABLE
#include "D3D12Device.hpp"
#include "RGLD3D12.hpp"
#include "RGLCommon.hpp"
#include "D3D12Swapchain.hpp"
#include "D3D12Surface.hpp"
#include "D3D12CommandQueue.hpp"
#include "D3D12Synchronization.hpp"
#include "D3D12ShaderLibrary.hpp"
#include "D3D12Buffer.hpp"
#include "D3D12RenderPipeline.hpp"
#include "D3D12Texture.hpp"
#include "D3D12Sampler.hpp"
#include "D3D12ComputePipeline.hpp"
#include <D3D12MemAlloc.h>
#include <DescriptorHeap.h>
#include <WinBase.h>

#if __has_include(<pix3.h>)
#include <pix3.h>
#define PIX_ENABLED 1
#endif

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

using namespace Microsoft::WRL;
using namespace std;

namespace RGL {

    ComPtr<IDXGIAdapter4> GetAdapter(bool useWarp)
    {
        ComPtr<IDXGIFactory4> dxgiFactory;
        UINT createFactoryFlags = 0;
#if defined(_DEBUG)
        createFactoryFlags = DXGI_CREATE_FACTORY_DEBUG;
#endif

        DX_CHECK(CreateDXGIFactory2(createFactoryFlags, IID_PPV_ARGS(&dxgiFactory)));

        ComPtr<IDXGIAdapter1> dxgiAdapter1;
        ComPtr<IDXGIAdapter4> dxgiAdapter4;

        // the software renderer
        if (useWarp)
        {
            DX_CHECK(dxgiFactory->EnumWarpAdapter(IID_PPV_ARGS(&dxgiAdapter1)));
            DX_CHECK(dxgiAdapter1.As(&dxgiAdapter4));
        }
        else
        {
            SIZE_T maxDedicatedVideoMemory = 0;
            for (UINT i = 0; dxgiFactory->EnumAdapters1(i, &dxgiAdapter1) != DXGI_ERROR_NOT_FOUND; ++i)
            {
                DXGI_ADAPTER_DESC1 dxgiAdapterDesc1;
                dxgiAdapter1->GetDesc1(&dxgiAdapterDesc1);

                // Check to see if the adapter can create a D3D12 device without actually 
                // creating it. The adapter with the largest dedicated video memory
                // is favored.
                // use DXGI_ADAPTER_FLAG_SOFTWARE to only select hardware devices
                if ((dxgiAdapterDesc1.Flags & DXGI_ADAPTER_FLAG_SOFTWARE) == 0 &&
                    SUCCEEDED(D3D12CreateDevice(dxgiAdapter1.Get(),
                        D3D_FEATURE_LEVEL_11_0, __uuidof(ID3D12Device), nullptr)) &&
                    dxgiAdapterDesc1.DedicatedVideoMemory > maxDedicatedVideoMemory)
                {
                    maxDedicatedVideoMemory = dxgiAdapterDesc1.DedicatedVideoMemory;
                    DX_CHECK(dxgiAdapter1.As(&dxgiAdapter4));
                }
            }
        }

        return dxgiAdapter4;
    }

    // create device from adapter
    // destroying a device causes all resources allocated on it to become invalid
    // should be destroyed after all resources have been destroyed (the validation layer will complain otherwise)
    ComPtr<ID3D12Device2> CreateDevice(ComPtr<IDXGIAdapter4> adapter)
    {
        ComPtr<ID3D12Device2> d3d12Device2;
        DX_CHECK(D3D12CreateDevice(adapter.Get(), D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&d3d12Device2)));

        // Enable debug messages in debug mode.
#if defined(_DEBUG)
        ComPtr<ID3D12InfoQueue> pInfoQueue;
        if (SUCCEEDED(d3d12Device2.As(&pInfoQueue)))
        {
            if (!PIXIsAttachedForGpuCapture()) {        // these can screw with PIX captures
                pInfoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_CORRUPTION, TRUE);
                pInfoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_ERROR, TRUE);
                pInfoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_WARNING, TRUE);
            }
            // Suppress whole categories of messages
            // uncomment to provide some
            //D3D12_MESSAGE_CATEGORY Categories[] = {};

            // Suppress messages based on their severity level
            D3D12_MESSAGE_SEVERITY Severities[] =
            {
                D3D12_MESSAGE_SEVERITY_INFO     // these don't indicate misuse of the API so we ignore them
            };

            // Suppress individual messages by their ID
            D3D12_MESSAGE_ID DenyIds[] = {
                D3D12_MESSAGE_ID_CLEARRENDERTARGETVIEW_MISMATCHINGCLEARVALUE,   // I'm really not sure how to avoid this message. Happens when clearing using a color other than the optimized clear color. If arbitrary clear colors are not desired, dont' suppress this message
                D3D12_MESSAGE_ID_MAP_INVALID_NULLRANGE,                         // This warning occurs when using capture frame while graphics debugging.
                D3D12_MESSAGE_ID_UNMAP_INVALID_NULLRANGE,                       // This warning occurs when using capture frame while graphics debugging.
            };

            D3D12_INFO_QUEUE_FILTER NewFilter = {};
            //NewFilter.DenyList.NumCategories = _countof(Categories);
            //NewFilter.DenyList.pCategoryList = Categories;
            NewFilter.DenyList.NumSeverities = _countof(Severities);
            NewFilter.DenyList.pSeverityList = Severities;
            NewFilter.DenyList.NumIDs = _countof(DenyIds);
            NewFilter.DenyList.pIDList = DenyIds;

            DX_CHECK(pInfoQueue->PushStorageFilter(&NewFilter));
        }
#endif

        return d3d12Device2;
    }


    RGLDevicePtr RGL::CreateDefaultDeviceD3D12() {
        ComPtr<IDXGIAdapter4> dxgiAdapter4 = GetAdapter(false); // for now, don't use WARP
        return std::make_shared<DeviceD3D12>(dxgiAdapter4);
    }

    DeviceD3D12::DeviceD3D12(decltype(adapter) adapter) : adapter(adapter), device(CreateDevice(adapter)), internalQueue(std::make_shared<CommandQueueD3D12>(this,QueueType::AllCommands)) {

        HANDLE deviceRemovedEvent = CreateEventW(NULL, FALSE, FALSE, NULL);
        assert(deviceRemovedEvent != NULL);
        DX_CHECK(device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&deviceRemovedFence)));
        deviceRemovedFence->SetEventOnCompletion(UINT64_MAX, deviceRemovedEvent);

        HANDLE waitHandle;
        RegisterWaitForSingleObject(
            &waitHandle,
            deviceRemovedEvent,
            RGL::RGLDeviceRemovedHandler,
            device.Get(), // Pass the device as our context
            INFINITE, // No timeout
            0 // No flags
        );

        internalCommandList = internalQueue->CreateCommandList();
        g_RTVDescriptorHeapSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

        D3D12MA::ALLOCATOR_DESC desc = {};
        desc.Flags = D3D12MA::ALLOCATOR_FLAG_DEFAULT_POOLS_NOT_ZEROED;
        desc.pDevice = device.Get();
        desc.pAdapter = adapter.Get();

        /*if (ENABLE_CPU_ALLOCATION_CALLBACKS)
        {
            g_AllocationCallbacks.pAllocate = &CustomAllocate;
            g_AllocationCallbacks.pFree = &CustomFree;
            g_AllocationCallbacks.pPrivateData = CUSTOM_ALLOCATION_PRIVATE_DATA;
            desc.pAllocationCallbacks = &g_AllocationCallbacks;
        }*/

        DX_CHECK(D3D12MA::CreateAllocator(&desc, &allocator));
        
        // create the descriptor heaps
        // these will be the only descriptor heaps
        // because it is more performant to have a single heap and suballocate it

        RTVHeap.emplace(device.Get(), D3D12_DESCRIPTOR_HEAP_TYPE_RTV, D3D12_DESCRIPTOR_HEAP_FLAG_NONE);
        DSVHeap.emplace(device.Get(), D3D12_DESCRIPTOR_HEAP_TYPE_DSV, D3D12_DESCRIPTOR_HEAP_FLAG_NONE);
        CBV_SRV_UAVHeap.emplace(device.Get(), D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE);
        SamplerHeap.emplace(device.Get(), D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER, D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE);

       
        {
            D3D12_INDIRECT_ARGUMENT_DESC argumentDescs[]{
            {.Type = D3D12_INDIRECT_ARGUMENT_TYPE_DRAW }
            };
            D3D12_COMMAND_SIGNATURE_DESC signature{
                .ByteStride = sizeof(D3D12_DRAW_ARGUMENTS),
                .NumArgumentDescs = std::size(argumentDescs),
                .pArgumentDescs = argumentDescs,
                .NodeMask = 0
            };
            device->CreateCommandSignature(&signature, nullptr, __uuidof(ID3D12CommandSignature), &multidrawSignature);
        }
        {
            D3D12_INDIRECT_ARGUMENT_DESC argumentDescs[]{
            {.Type = D3D12_INDIRECT_ARGUMENT_TYPE_DRAW_INDEXED}
            };
            D3D12_COMMAND_SIGNATURE_DESC signature{
                .ByteStride = sizeof(D3D12_DRAW_INDEXED_ARGUMENTS),
                .NumArgumentDescs = std::size(argumentDescs),
                .pArgumentDescs = argumentDescs,
                .NodeMask = 0
            };
            device->CreateCommandSignature(&signature, nullptr, __uuidof(ID3D12CommandSignature), &multidrawIndexedSignature);
        }
        {
            D3D12_INDIRECT_ARGUMENT_DESC argumentDescs[]{
                {
                    .Type = D3D12_INDIRECT_ARGUMENT_TYPE_DISPATCH
                }
            };
            D3D12_COMMAND_SIGNATURE_DESC signature{
               .ByteStride = sizeof(D3D12_DISPATCH_ARGUMENTS),
               .NumArgumentDescs = std::size(argumentDescs),
               .pArgumentDescs = argumentDescs,
               .NodeMask = 0
            };
            device->CreateCommandSignature(&signature, nullptr, __uuidof(ID3D12CommandSignature), &dispatchIndirectSignature);
        }
    }

    DeviceD3D12::~DeviceD3D12() {
        // all comptrs decrement to 0 if not held elsewhere and are released
    }

    std::string DeviceD3D12::GetBrandString() {
        DXGI_ADAPTER_DESC desc;
        adapter->GetDesc(&desc);

        std::wstring wstr(desc.Description);

        //setup converter
        std::string result;
        result.resize(wstr.size()+1);
        wcstombs_s(nullptr,result.data(), result.size(), wstr.data(), _TRUNCATE);
        return result;
    }
    RGLSwapchainPtr RGL::DeviceD3D12::CreateSwapchain(RGLSurfacePtr surface, RGLCommandQueuePtr presentQueue, int width, int height)
    {
        return std::make_shared<SwapchainD3D12>(shared_from_this(), std::static_pointer_cast<SurfaceD3D12>(surface), width, height, std::static_pointer_cast<CommandQueueD3D12>(presentQueue));
    }
    RGLPipelineLayoutPtr RGL::DeviceD3D12::CreatePipelineLayout(const PipelineLayoutDescriptor& desc)
    {
        return std::make_shared<PipelineLayoutD3D12>(shared_from_this(),desc);
    }
    RGLRenderPipelinePtr RGL::DeviceD3D12::CreateRenderPipeline(const RenderPipelineDescriptor& desc)
    {
        return std::make_shared<RenderPipelineD3D12>(shared_from_this(), desc);
    }
    RGLComputePipelinePtr DeviceD3D12::CreateComputePipeline(const ComputePipelineDescriptor& desc)
    {
        return std::make_shared<ComputePipelineD3D12>(shared_from_this(), desc);
    }
    RGLShaderLibraryPtr DeviceD3D12::CreateShaderLibraryFromName(const std::string_view& name)
    {
        FatalError("CreateShaderLibraryFromName not implemented");
        return RGLShaderLibraryPtr();
    }
    RGLShaderLibraryPtr RGL::DeviceD3D12::CreateDefaultShaderLibrary()
    {
        return std::make_shared<ShaderLibraryD3D12>();
    }
    RGLShaderLibraryPtr RGL::DeviceD3D12::CreateShaderLibraryFromBytes(const std::span<const uint8_t> bytes)
    {
        return std::make_shared<ShaderLibraryD3D12>(bytes);
    }
    RGLShaderLibraryPtr RGL::DeviceD3D12::CreateShaderLibrarySourceCode(const std::string_view sourcecode, const FromSourceConfig& config)
    {
        return std::make_shared<ShaderLibraryD3D12>(sourcecode, config);
    }
    RGLShaderLibraryPtr RGL::DeviceD3D12::CreateShaderLibraryFromPath(const std::filesystem::path& file)
    {
        return std::make_shared<ShaderLibraryD3D12>(file);
    }

    RGLBufferPtr RGL::DeviceD3D12::CreateBuffer(const BufferConfig& config)
    {
        return std::make_shared<BufferD3D12>(shared_from_this(), config);
    }

    RGLTexturePtr DeviceD3D12::CreateTextureWithData(const TextureConfig& config, untyped_span bytes)
    {
        return std::make_shared<TextureD3D12>(shared_from_this(), config, bytes);
    }

    RGLTexturePtr DeviceD3D12::CreateTexture(const TextureConfig& config)
    {
        return std::make_shared<TextureD3D12>(shared_from_this(), config);
    }

    RGLSamplerPtr DeviceD3D12::CreateSampler(const SamplerConfig& config)
    {
        return std::make_shared<SamplerD3D12>(shared_from_this(),config);
    }

    DeviceData DeviceD3D12::GetDeviceData()
    {
        return {
            .d3d12Data = {
                .device = device.Get()
            }
        };
    }

    TextureView DeviceD3D12::GetGlobalBindlessTextureHeap() const
    {

        return TextureView{ {
            .dsvIDX = 0,
            .rtvIDX = 0,
            .srvIDX = 0,
            .uavIDX = 0,
            .representsBindless = true,
            .parentResource = nullptr,      // bindless must set barriers elsewhere
            .coveredMips = ALL_MIPS,
            .coveredLayers = ALL_LAYERS
        } };
    }

    RGLCommandQueuePtr RGL::DeviceD3D12::CreateCommandQueue(QueueType type)
    {
        return std::make_shared<CommandQueueD3D12>(this, type);
    }

    RGLFencePtr RGL::DeviceD3D12::CreateFence(bool preSignaled)
    {
        return std::make_shared<FenceD3D12>(shared_from_this(), preSignaled);
    }

    void RGL::DeviceD3D12::BlockUntilIdle()
    {
        Flush();
    }

    size_t DeviceD3D12::GetTotalVRAM() const
    {
        DXGI_QUERY_VIDEO_MEMORY_INFO meminfo;
        adapter->QueryVideoMemoryInfo(0, DXGI_MEMORY_SEGMENT_GROUP_LOCAL, &meminfo);
        return meminfo.Budget;
    }

    size_t DeviceD3D12::GetCurrentVRAMInUse() const
    {
        DXGI_QUERY_VIDEO_MEMORY_INFO meminfo;
        adapter->QueryVideoMemoryInfo(0, DXGI_MEMORY_SEGMENT_GROUP_LOCAL, &meminfo);
        return meminfo.CurrentUsage;
    }

    void DeviceD3D12::Flush() {
        internalQueue->Flush();
    }
}

#endif
