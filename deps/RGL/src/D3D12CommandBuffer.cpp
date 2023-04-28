#if RGL_DX12_AVAILABLE
#include "D3D12CommandBuffer.hpp"
#include "D3D12CommandQueue.hpp"
#include "D3D12Synchronization.hpp"
#include <directx/d3dx12.h>
#include "D3D12Texture.hpp"
#include "D3D12RenderPipeline.hpp"
#include "D3D12Buffer.hpp"
#include "D3D12Device.hpp"
#include "D3D12Sampler.hpp"
#include "D3D12RenderPass.hpp"
#include "D3D12ComputePipeline.hpp"

namespace RGL {

	CommandBufferD3D12::CommandBufferD3D12(decltype(owningQueue) owningQueue) : owningQueue(owningQueue)
	{
		commandList = owningQueue->CreateCommandList();
	}
	void CommandBufferD3D12::Reset()
	{
		ID3D12CommandAllocator* commandAllocator;
		UINT dataSize = sizeof(commandAllocator);
		DX_CHECK(commandList->GetPrivateData(__uuidof(ID3D12CommandAllocator), &dataSize, &commandAllocator));
		if (!ended) {
			DX_CHECK(commandList->Close());
		}
		DX_CHECK(commandAllocator->Reset());	// gotta reset this too, otherwise we leak
		DX_CHECK(commandList->Reset(commandAllocator, nullptr));
		ended = false;
	}
	void CommandBufferD3D12::Begin()
	{
		commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	}
	void CommandBufferD3D12::End()
	{
		DX_CHECK(commandList->Close());
		ended = true;
	}
	void CommandBufferD3D12::BeginRendering(RGLRenderPassPtr renderPass)
	{
		currentRenderPass = std::static_pointer_cast<RenderPassD3D12>(renderPass);

		const auto nrtvs = currentRenderPass->config.attachments.size();
		stackarray(rtvs, CD3DX12_CPU_DESCRIPTOR_HANDLE,nrtvs);

		uint32_t i = 0;
		for (const auto& attachment : currentRenderPass->config.attachments) {
			auto tx = static_cast<TextureD3D12*>(currentRenderPass->textures[i]);

			Assert(tx->rtvAllocated(),"This texture was not allocated as a render target!");
			
			auto rtv = tx->owningDevice->RTVHeap->GetCpuHandle(tx->rtvIDX);

			commandList->ClearRenderTargetView(rtv, attachment.clearColor.data(), 0, nullptr);

			rtvs[i] = rtv;

			i++;
		}

		//depth stencil
		D3D12_CPU_DESCRIPTOR_HANDLE dsv;
		D3D12_CPU_DESCRIPTOR_HANDLE* dsvptr = nullptr;
		{
			if (currentRenderPass->depthTexture) {
				auto tx = static_cast<TextureD3D12*>(currentRenderPass->depthTexture);
				Assert(tx->dsvAllocated(), "Texture was not allocated as a depth stencil!");
				dsv = tx->owningDevice->DSVHeap->GetCpuHandle(tx->dsvIDX);
				dsvptr = &dsv;
				if (currentRenderPass->config.depthAttachment->loadOp == LoadAccessOperation::Clear) {
					commandList->ClearDepthStencilView(dsv, D3D12_CLEAR_FLAG_DEPTH, currentRenderPass->config.depthAttachment->clearColor[0], 0, 0, nullptr);
				}
			}
		}

		// bind the targets
		commandList->OMSetRenderTargets(nrtvs, rtvs, FALSE, dsvptr);
	}
	void CommandBufferD3D12::EndRendering()
	{
		currentRenderPass = nullptr;
		currentRenderPipeline = nullptr;
	}
	void CommandBufferD3D12::BindRenderPipeline(RGLRenderPipelinePtr in_pipeline)
	{
		currentRenderPipeline = std::static_pointer_cast<RenderPipelineD3D12>(in_pipeline);
		commandList->SetPipelineState(currentRenderPipeline->pipelineState.Get());
		commandList->SetGraphicsRootSignature(currentRenderPipeline->pipelineLayout->rootSignature.Get());
	}
	void CommandBufferD3D12::BeginCompute(RGLComputePipelinePtr in_pipeline)
	{
		currentComputePipeline = std::static_pointer_cast<ComputePipelineD3D12>(in_pipeline);
		commandList->SetPipelineState(currentComputePipeline->pipelineState.Get());
		commandList->SetComputeRootSignature(currentComputePipeline->pipelineLayout->rootSignature.Get());
	}
	void CommandBufferD3D12::EndCompute()
	{
		currentComputePipeline.reset();
	}
	void CommandBufferD3D12::DispatchCompute(uint32_t threadsX, uint32_t threadsY, uint32_t threadsZ)
	{
		commandList->Dispatch(threadsX, threadsY, threadsZ);
	}
	void CommandBufferD3D12::BindBuffer(RGLBufferPtr buffer, uint32_t bindingOffset, uint32_t offsetIntoBuffer)
	{
		auto casted = std::static_pointer_cast<BufferD3D12>(buffer);
		const auto layout = currentRenderPipeline->pipelineLayout;
		const auto bindPoint = layout->slotForBufferIdx(bindingOffset);
		if (layout->bufferIdxIsUAV(bindingOffset)) {
			commandList->SetGraphicsRootUnorderedAccessView(bindPoint, casted->vertexBufferView.BufferLocation + offsetIntoBuffer);
		}
		else {
			commandList->SetGraphicsRootShaderResourceView(bindPoint, casted->vertexBufferView.BufferLocation + offsetIntoBuffer);
		}
	}
	void CommandBufferD3D12::BindComputeBuffer(RGLBufferPtr buffer, uint32_t bindingOffset, uint32_t offsetIntoBuffer)
	{
		auto casted = std::static_pointer_cast<BufferD3D12>(buffer);
		const auto currentLayout = currentComputePipeline->pipelineLayout;
		const auto slotidx = currentLayout->slotForBufferIdx(bindingOffset);
		if (currentLayout->bufferIdxIsUAV(bindingOffset)) {
			commandList->SetComputeRootUnorderedAccessView(slotidx, casted->vertexBufferView.BufferLocation + offsetIntoBuffer);
		}
		else {
			commandList->SetComputeRootShaderResourceView(slotidx, casted->vertexBufferView.BufferLocation + offsetIntoBuffer);

		}
	}
	void CommandBufferD3D12::SetVertexBuffer(RGLBufferPtr buffer, const VertexBufferBinding& bindingInfo)
	{
		commandList->IASetVertexBuffers(bindingInfo.bindingPosition, 1, &std::static_pointer_cast<BufferD3D12>(buffer)->vertexBufferView + bindingInfo.offsetIntoBuffer);
	}
	void CommandBufferD3D12::SetVertexBytes(const untyped_span data, uint32_t offset)
	{
		auto size = data.size() / 4 + (data.size() % 4 != 0 ? 1 : 0);
		commandList->SetGraphicsRoot32BitConstants(offset, size, data.data(), 0);
	}
	void CommandBufferD3D12::SetFragmentBytes(const untyped_span data, uint32_t offset)
	{
		// d3d does not make a distinction
		SetVertexBytes(data, offset);
	}
	void CommandBufferD3D12::SetComputeBytes(const untyped_span data, uint32_t offset)
	{
		auto size = data.size() / 4 + (data.size() % 4 != 0 ? 1 : 0);
		commandList->SetComputeRoot32BitConstants(offset, size, data.data(), 0);
	}
	void CommandBufferD3D12::SetIndexBuffer(RGLBufferPtr buffer)
	{
		commandList->IASetIndexBuffer(&(std::static_pointer_cast<BufferD3D12>(buffer)->indexBufferView));
	}
	void CommandBufferD3D12::SetVertexSampler(RGLSamplerPtr sampler, uint32_t index)
	{
		SetFragmentSampler(sampler, index);
	}
	void CommandBufferD3D12::SetFragmentSampler(RGLSamplerPtr sampler, uint32_t index)
	{
		index = currentRenderPipeline->pipelineLayout->slotForSamplerIdx(index);
		auto thisSampler = std::static_pointer_cast<SamplerD3D12>(sampler);
		auto& samplerHeap = thisSampler->owningDevice->SamplerHeap;
		ID3D12DescriptorHeap* heapForThis = samplerHeap->Heap();
		auto handle = samplerHeap->GetGpuHandle(thisSampler->descriptorIndex);
		commandList->SetDescriptorHeaps(1, &heapForThis);
		commandList->SetGraphicsRootDescriptorTable(index, handle);
	}
	void CommandBufferD3D12::SetVertexTexture(const ITexture* texture, uint32_t index)
	{
		SetFragmentTexture(texture, index);
	}
	void CommandBufferD3D12::SetFragmentTexture(const ITexture* texture, uint32_t index)
	{
		index += currentRenderPipeline->pipelineLayout->slotForSamplerIdx(index);;
		auto thisTexture = static_cast<const TextureD3D12*>(texture);
		auto& heapForThis = thisTexture->owningDevice->CBV_SRV_UAVHeap.value();
		auto ptr = heapForThis.Heap();
		commandList->SetDescriptorHeaps(1, &ptr);
		commandList->SetGraphicsRootDescriptorTable(index, heapForThis.GetGpuHandle(thisTexture->srvIDX));
	}
	void CommandBufferD3D12::SetCombinedTextureSampler(RGLSamplerPtr sampler, const ITexture* texture, uint32_t index)
	{
		// combined image samplers are indexed in pairs (sampler, texture) starting after the constants
		// the index is into the descriptor table array
		// example (if firstSamplerIdx is 1):
		// 0 -> (1,2)
		// 1 -> (3,4)
		// 2 -> (5,6)
		// etc
		const auto pipelineLayout = currentRenderPipeline->pipelineLayout;
		auto thisSampler = std::static_pointer_cast<SamplerD3D12>(sampler);
		auto thisTexture = static_cast<const TextureD3D12*>(texture);
		auto& srvheap = thisTexture->owningDevice->CBV_SRV_UAVHeap;
		auto& samplerHeap = thisSampler->owningDevice->SamplerHeap;
		ID3D12DescriptorHeap* heapForThis[2] = { samplerHeap->Heap(), srvheap->Heap()};
		commandList->SetDescriptorHeaps(std::size(heapForThis), heapForThis);

		const auto samplerSlot = pipelineLayout->slotForSamplerIdx(index);
		const auto textureSlot = pipelineLayout->slotForTextureIdx(index);
		commandList->SetGraphicsRootDescriptorTable(samplerSlot, samplerHeap->GetGpuHandle(thisSampler->descriptorIndex));
		commandList->SetGraphicsRootDescriptorTable(textureSlot, srvheap->GetGpuHandle(thisTexture->srvIDX));
	}
	void CommandBufferD3D12::Draw(uint32_t nVertices, const DrawInstancedConfig& config)
	{
		commandList->DrawInstanced(nVertices, config.nInstances, config.startVertex, config.firstInstance);
	}
	void CommandBufferD3D12::DrawIndexed(uint32_t nIndices, const DrawIndexedInstancedConfig& config)
	{
		commandList->DrawIndexedInstanced(nIndices, config.nInstances, config.firstIndex, config.startVertex, config.firstInstance);
	}
	void CommandBufferD3D12::SetViewport(const Viewport& viewport)
	{
		D3D12_VIEWPORT m_Viewport{
			.TopLeftX = viewport.x,
			.TopLeftY = viewport.y,
			.Width = viewport.width,
			.Height = viewport.height,
			.MinDepth = viewport.minDepth,
			.MaxDepth = viewport.maxDepth,
		};

		commandList->RSSetViewports(1, &m_Viewport);
	}
	void CommandBufferD3D12::SetScissor(const Rect& scissor)
	{
		D3D12_RECT m_ScissorRect{ CD3DX12_RECT{
			scissor.offset[0], 
			scissor.offset[1], 
			static_cast<LONG>(scissor.offset[0] + scissor.extent[0]), 
			static_cast<LONG>(scissor.offset[1] + scissor.extent[1])
		}};

		commandList->RSSetScissorRects(1, &m_ScissorRect);
	}
	void CommandBufferD3D12::SetRenderPipelineBarrier(const BarrierConfig& config)
	{
		auto totalBarriers = config.buffers.size() + config.textures.size();
		stackarray(barriers, D3D12_RESOURCE_BARRIER, totalBarriers);

		uint32_t i = 0;
		for (const auto& bufferBase : config.buffers) {
			auto buffer = std::static_pointer_cast<BufferD3D12>(bufferBase);
			barriers[i] = {
				.Type = D3D12_RESOURCE_BARRIER_TYPE_UAV,
				.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE,
				.UAV = {
					.pResource = buffer->buffer.Get(),
				}
			};
			i++;
		}

		i = config.buffers.size();
		for (const auto& textureBase: config.textures) {
			auto texture = std::static_pointer_cast<TextureD3D12>(textureBase);
			barriers[i] = {
				.Type = D3D12_RESOURCE_BARRIER_TYPE_UAV,
				.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE,
				.UAV = {
					.pResource = texture->texture.Get()
				}
			};
		}

		commandList->ResourceBarrier(totalBarriers, barriers);
	}
	void CommandBufferD3D12::CopyTextureToBuffer(RGL::ITexture* sourceTexture, const Rect& sourceRect, size_t offset, RGLBufferPtr desetBuffer)
	{
		auto casted = static_cast<TextureD3D12*>(sourceTexture);
		auto castedDest = std::static_pointer_cast<BufferD3D12>(desetBuffer);
		D3D12_TEXTURE_COPY_LOCATION destination{
			.pResource = castedDest->buffer.Get(),
			.Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT
		};

		D3D12_RESOURCE_DESC srcDesc = casted->texture->GetDesc();
		castedDest->owningDevice->device->GetCopyableFootprints(
			&srcDesc, 0, 1, 0,
			&destination.PlacedFootprint,
			NULL, NULL, NULL
		);

		// set bounds
		destination.PlacedFootprint.Footprint.Width = sourceRect.extent[0];
		destination.PlacedFootprint.Footprint.Height = sourceRect.extent[1];

		D3D12_TEXTURE_COPY_LOCATION source{
			.pResource = casted->texture.Get(),
			.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX,
			.SubresourceIndex = 0,
		};

		D3D12_BOX srcBox{
			.left = static_cast<UINT>(sourceRect.offset[0]),
			.top = static_cast<UINT>(sourceRect.offset[1]),
			.front = 0,
			.right = static_cast<UINT>(sourceRect.offset[0] + sourceRect.extent[0]),
			.bottom = static_cast<UINT>(sourceRect.offset[1] + sourceRect.extent[1]),
			.back = 1,
		};

		commandList->CopyTextureRegion(&destination, 0, 0, 0, &source, &srcBox);
	}
	void CommandBufferD3D12::TransitionResource(const ITexture* texture, RGL::ResourceLayout current, RGL::ResourceLayout target, TransitionPosition position)
	{
		auto casted = static_cast<const TextureD3D12*>(texture);
		auto beforeState = rgl2d3d12resourcestate(current);
		auto afterState = rgl2d3d12resourcestate(target);
		CD3DX12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(
			casted->texture.Get(),
			beforeState, 
			afterState
		);
		commandList->ResourceBarrier(1, &barrier);
	}
	void CommandBufferD3D12::Commit(const CommitConfig& config)
	{
		owningQueue->ExecuteCommandList(commandList);
		if (config.signalFence) {
			auto d3d12fence = std::static_pointer_cast<FenceD3D12>(config.signalFence);
			owningQueue->m_d3d12CommandQueue->Signal(d3d12fence->fence.Get(), 1);	// 1 because we emulate binary vulkan fences
		}
	}
	void CommandBufferD3D12::ExecuteIndirectIndexed(const IndirectConfig& config)
	{
		auto buffer = std::static_pointer_cast<BufferD3D12>(config.indirectBuffer);
		auto sig = buffer->owningDevice->multidrawIndexedSignature;
		commandList->ExecuteIndirect(
			sig.Get(),
			config.nDraws,
			buffer->buffer.Get(),
			config.offsetIntoBuffer,
			nullptr,
			0
		);
	}
	void CommandBufferD3D12::ExecuteIndirect(const IndirectConfig& config)
	{
		auto buffer = std::static_pointer_cast<BufferD3D12>(config.indirectBuffer);
		auto sig = buffer->owningDevice->multidrawSignature;
		commandList->ExecuteIndirect(
			sig.Get(),
			config.nDraws,
			buffer->buffer.Get(),
			config.offsetIntoBuffer,
			nullptr,
			0
		);
	}
}

#endif
