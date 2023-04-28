#if RGL_DX12_AVAILABLE
#include "D3D12CommandQueue.hpp"
#include "RGLD3D12.hpp"
#include "D3D12Device.hpp"
#include "D3D12CommandBuffer.hpp"

namespace RGL {
    D3D12_COMMAND_LIST_TYPE rgl2d3d12cmdlist(QueueType type) {
        switch (type) {
        case decltype(type)::AllCommands:
            return D3D12_COMMAND_LIST_TYPE_DIRECT;
        case decltype(type)::CopyOnly:
            return D3D12_COMMAND_LIST_TYPE_COPY;
        case decltype(type)::ComputeOnly:
            return D3D12_COMMAND_LIST_TYPE_COMPUTE;
        default:
            FatalError("Invalid Queuetype");
        }
    }

    RGLCommandBufferPtr CommandQueueD3D12::CreateCommandBuffer()
	{
		return std::make_shared<CommandBufferD3D12>(shared_from_this());
	}
    CommandQueueD3D12::CommandQueueD3D12(ComPtr<ID3D12Device2> device, QueueType type)
        : m_FenceValue(0),
        m_CommandListType(rgl2d3d12cmdlist(type)),
        m_d3d12Device(device)
    {
        D3D12_COMMAND_QUEUE_DESC desc = {};
        desc.Type = m_CommandListType;
        desc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;
        desc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
        desc.NodeMask = 0;

        DX_CHECK(m_d3d12Device->CreateCommandQueue(&desc, IID_PPV_ARGS(&m_d3d12CommandQueue)));
        DX_CHECK(m_d3d12Device->CreateFence(m_FenceValue, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_d3d12Fence)));

        m_FenceEvent = ::CreateEvent(NULL, FALSE, FALSE, NULL);
        assert(m_FenceEvent && "Failed to create fence event handle.");
    }

    Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList2> CommandQueueD3D12::CreateCommandList()
    {
        Microsoft::WRL::ComPtr<ID3D12CommandAllocator> allocator;
        DX_CHECK(m_d3d12Device->CreateCommandAllocator(m_CommandListType, IID_PPV_ARGS(&allocator)));
        Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList2> commandList;
        DX_CHECK(m_d3d12Device->CreateCommandList(0, m_CommandListType, allocator.Get(), nullptr, IID_PPV_ARGS(&commandList)));

        // Associate the command allocator with the command list so that it can be
        // retrieved when the command list is executed.
        DX_CHECK(commandList->SetPrivateDataInterface(__uuidof(ID3D12CommandAllocator), allocator.Get()));

        // It should be noted that when assigning a COM object to the private data of a ID3D12Object object using the ID3D12Object::SetPrivateDataInterface method, the internal reference counter of the assigned COM object is incremented. The ref counter of the assigned COM object is only decremented if either the owning ID3D12Object object is destroyed or the instance of the COM object with the same interface is replaced with another COM object of the same interface or a NULL pointer. 

        return commandList;
    }

    QueueData CommandQueueD3D12::GetQueueData()
    {
        return {
            .d3d12Data = {
                .commandQueue = m_d3d12CommandQueue.Get()
            }
        };
    }


    uint64_t CommandQueueD3D12::ExecuteCommandList(Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList2> commandList)
    {

        ID3D12CommandAllocator* commandAllocator;
        UINT dataSize = sizeof(commandAllocator);
        DX_CHECK(commandList->GetPrivateData(__uuidof(ID3D12CommandAllocator), &dataSize, &commandAllocator));
        // You should be aware that retrieving a COM pointer of a COM object associated with the private data of the ID3D12Object object will also increment the reference counter of that COM object.

        ID3D12CommandList* const ppCommandLists[] = {
            commandList.Get()
        };

        m_d3d12CommandQueue->ExecuteCommandLists(std::size(ppCommandLists), ppCommandLists);
        uint64_t fenceValue = Signal();


        // The ownership of the command allocator has been transferred to the ComPtr
        // in the command allocator queue. It is safe to release the reference 
        // in this temporary COM pointer here.
        commandAllocator->Release();

        return fenceValue;
    }

    uint64_t CommandQueueD3D12::Signal()
    {
        uint64_t fenceValue = ++m_FenceValue;
        m_d3d12CommandQueue->Signal(m_d3d12Fence.Get(), fenceValue);
        return fenceValue;
    }

    bool CommandQueueD3D12::IsFenceComplete(uint64_t fenceValue)
    {
        return m_d3d12Fence->GetCompletedValue() >= fenceValue;
    }

    void CommandQueueD3D12::WaitForFenceValue(uint64_t fenceValue)
    {
        if (!IsFenceComplete(fenceValue))
        {
            m_d3d12Fence->SetEventOnCompletion(fenceValue, m_FenceEvent);
            ::WaitForSingleObject(m_FenceEvent, DWORD_MAX);
        }
    }

    void CommandQueueD3D12::Flush()
    {
        WaitForFenceValue(Signal());
    }
}
#endif
