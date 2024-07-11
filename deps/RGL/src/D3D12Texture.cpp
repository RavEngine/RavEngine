#if RGL_DX12_AVAILABLE
#include "D3D12Texture.hpp"
#include <directx/d3dx12.h>
#include "D3D12Device.hpp"
#include "D3D12CommandQueue.hpp"
#include <D3D12MemAlloc.h>
#include <ResourceUploadBatch.h>

using namespace Microsoft::WRL;

namespace RGL {

	DXGI_FORMAT typelessForDS(DXGI_FORMAT format) {
		switch (format) {
		case DXGI_FORMAT_D32_FLOAT:
			return DXGI_FORMAT_R32_TYPELESS;
		case DXGI_FORMAT_D24_UNORM_S8_UINT:
			return DXGI_FORMAT_R24G8_TYPELESS;
		default:
			return format;	// otherwise just passthrough
		}
	}

	DXGI_FORMAT typelessForSRV(DXGI_FORMAT format) {
		switch (format) {
		case DXGI_FORMAT_D32_FLOAT:
			return DXGI_FORMAT_R32_FLOAT;
		default:
			return format;	//otherwise just passthrough
		}
	}

	TextureD3D12::TextureD3D12(decltype(texture) image, const Dimension& size, decltype(rtvIDX) offset, decltype(owningDevice) device) : texture(image), ITexture(size), rtvIDX(offset), owningDevice(device)
	{
		nativeState = D3D12_RESOURCE_STATE_PRESENT;

		DirectX::ResourceUploadBatch impromptuTransition(owningDevice->device.Get());
		impromptuTransition.Begin();

		impromptuTransition.Transition(texture.Get(), D3D12_RESOURCE_STATE_COMMON, nativeState);

		auto finish = impromptuTransition.End(owningDevice->internalQueue->m_d3d12CommandQueue.Get());
		finish.wait();
	}
	TextureD3D12::TextureD3D12(decltype(texture) image, const TextureConfig& config, std::shared_ptr<IDevice> indevice, D3D12_RESOURCE_STATES nativeStateOverride) : owningDevice(std::static_pointer_cast<DeviceD3D12>(indevice)), ITexture({config.width, config.height}), texture(image)
	{
		// make the heap and SRV 
		bool canBeshadervisible = config.usage.Sampled;
		auto format = rgl2dxgiformat_texture(config.format);
		PlaceInHeaps(owningDevice, format, config);
		nativeState = nativeStateOverride;
	}
	TextureD3D12::TextureD3D12(decltype(owningDevice) owningDevice, const TextureConfig& config, untyped_span bytes) : TextureD3D12(owningDevice, config)
	{

		DirectX::ResourceUploadBatch upload(owningDevice->device.Get());

		upload.Begin();

		D3D12_SUBRESOURCE_DATA initData = { bytes.data(), bytes.size() / config.height, bytes.size()};
		upload.Transition(texture.Get(),
			nativeState,
			D3D12_RESOURCE_STATE_COPY_DEST
		);
		upload.Upload(texture.Get(), 0, &initData, 1);

		constexpr static auto endState = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE | D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE;

		upload.Transition(texture.Get(),
			D3D12_RESOURCE_STATE_COPY_DEST,
			endState
			);

		nativeState = endState;

		auto finish = upload.End(owningDevice->internalQueue->m_d3d12CommandQueue.Get());
		finish.wait();

	}
	TextureD3D12::TextureD3D12(decltype(owningDevice) owningDevice, const TextureConfig& config) : owningDevice(owningDevice), ITexture({ config.width,config.height })
	{
		auto format = rgl2dxgiformat_texture(config.format);
		auto mainResourceFormat = format;

		const bool isDS = (config.aspect.HasDepth || config.aspect.HasStencil);

		// we cannot sample depth textures directly
		// instead, we have to create this resource as Typeless, and then cast it to compatbile
		// formats for the SRVs
		if (isDS && config.usage.Sampled) {
			mainResourceFormat = typelessForDS(format);
		}

		D3D12_RESOURCE_DESC resourceDesc = {};
		resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
		resourceDesc.Alignment = 0;
		resourceDesc.Width = config.width;
		resourceDesc.Height = config.height;
		resourceDesc.DepthOrArraySize = config.arrayLayers;
		resourceDesc.MipLevels = config.mipLevels;
		resourceDesc.Format = mainResourceFormat;
		resourceDesc.SampleDesc.Count = 1;
		resourceDesc.SampleDesc.Quality = 0;
		resourceDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
		resourceDesc.Flags = config.usage.Storage ? D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS : D3D12_RESOURCE_FLAG_NONE;

		numMips = config.mipLevels;
		numLayers = config.arrayLayers;


		D3D12_CLEAR_VALUE optimizedClearValue = {
			.Format = format,
		};

		nativeState = D3D12_RESOURCE_STATE_COMMON;
		if (isDS) {
			resourceDesc.Flags |= D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;
			optimizedClearValue.DepthStencil = { config.optimizedClearValue[0], 0};
			if (!config.usage.Sampled) {
				nativeState |= D3D12_RESOURCE_STATE_DEPTH_WRITE;
			}
		}

		if (config.usage.ColorAttachment) {
			resourceDesc.Flags |= D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;
			std::fill(optimizedClearValue.Color, optimizedClearValue.Color + std::size(optimizedClearValue.Color), 0);
			//nativeState |= D3D12_RESOURCE_STATE_RENDER_TARGET;
		}


		D3D12MA::ALLOCATION_DESC allocDesc = {};
		allocDesc.HeapType = D3D12_HEAP_TYPE_DEFAULT;

		// allocate the resource

		if (nativeState == D3D12_RESOURCE_STATE_COMMON) {
			nativeState = D3D12_RESOURCE_STATE_GENERIC_READ;
		}

		HRESULT hr = owningDevice->allocator->CreateResource(
			&allocDesc, &resourceDesc,
			nativeState, (resourceDesc.Flags & D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL || resourceDesc.Flags & D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET) ? &optimizedClearValue : nullptr,
			&allocation, IID_PPV_ARGS(&texture));

		std::wstring wide;
		wide.resize(config.debugName.data() == nullptr ? 0 : config.debugName.length());
		MultiByteToWideChar(CP_UTF8, 0, config.debugName.data(), -1, wide.data(), wide.size());
		texture->SetName(wide.data());

		if (config.debugName.data() != nullptr) {
			debugName = config.debugName;
		}

		// add the resource to the appropriate heaps
		PlaceInHeaps(owningDevice, format, config);
		
	}
	void TextureD3D12::PlaceInHeaps(const std::shared_ptr<RGL::DeviceD3D12>& owningDevice, DXGI_FORMAT format, const RGL::TextureConfig& config)
	{
		const bool isDS = (config.aspect.HasDepth || config.aspect.HasStencil);
		mipHeapIndicesSRV.reserve(config.mipLevels);
		mipHeapIndicesUAV.reserve(config.mipLevels);
		mipHeapIndicesRTV.reserve(config.mipLevels);

		if (isDS) {
			dsvIDX = owningDevice->DSVHeap->AllocateSingle();
			D3D12_DEPTH_STENCIL_VIEW_DESC desc{
				.Format = format,
				.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D,
			};
			auto handle = owningDevice->DSVHeap->GetCpuHandle(dsvIDX);
			owningDevice->device->CreateDepthStencilView(texture.Get(), &desc, handle);
		}
		if (config.usage.ColorAttachment) {
			auto createRTV = [owningDevice, &format, this](UINT& outRTV, UINT mip, bool allMips) {
				outRTV = owningDevice->RTVHeap->AllocateSingle();
				D3D12_RENDER_TARGET_VIEW_DESC desc{
				.Format = format,
				.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D,
				.Texture2D = {
					.MipSlice = mip,
				}
				};
				auto handle = owningDevice->RTVHeap->GetCpuHandle(outRTV);
				owningDevice->device->CreateRenderTargetView(texture.Get(), &desc, handle);
			};
			
			createRTV(rtvIDX, 0, true);

			for (UINT i = 0; i < config.mipLevels; i++) {
				auto& handle = mipHeapIndicesRTV.emplace_back();
				createRTV(handle, i, false);
			}
		}
		if (config.usage.Sampled) {
			if (isDS) {
				// we need to change the format again because depth formats are not allowed for use in SRVs
				format = typelessForSRV(format);
			}
			auto createSRV = [owningDevice,&format,this](UINT& outSRV, UINT mip, bool allMips, bool isCube = false) {
				outSRV = owningDevice->CBV_SRV_UAVHeap->AllocateSingle();
				D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
				srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
				srvDesc.Format = format;
				srvDesc.ViewDimension = isCube ? D3D12_SRV_DIMENSION_TEXTURECUBE : D3D12_SRV_DIMENSION_TEXTURE2D;
				srvDesc.Texture2D.MipLevels = allMips ? -1 : 1;	// all levels or 1 level
				srvDesc.Texture2D.MostDetailedMip = mip;

				auto handle = owningDevice->CBV_SRV_UAVHeap->GetCpuHandle(outSRV);
				owningDevice->device->CreateShaderResourceView(texture.Get(), &srvDesc, handle);
			};
			createSRV(srvIDX, 0, true, config.isCubemap);
			for (UINT i = 0; i < config.mipLevels; i++) {
				auto& handle = mipHeapIndicesSRV.emplace_back();
				createSRV(handle, i, false);
			}
		}
		if (config.usage.Storage) {
			auto createUAV = [&format,owningDevice, this](UINT& outHandle, UINT mip) {
				outHandle = owningDevice->CBV_SRV_UAVHeap->AllocateSingle();
				D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc = {
					.Format = format,
					.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D,
					.Texture2D = {
						.MipSlice = mip,
						.PlaneSlice = 0
					}
				};
				auto handle = owningDevice->CBV_SRV_UAVHeap->GetCpuHandle(outHandle);
				owningDevice->device->CreateUnorderedAccessView(texture.Get(), nullptr, &uavDesc, handle);
			};
			createUAV(uavIDX, 0);
			for (UINT i = 0; i < config.mipLevels; i++) {
				auto& handle = mipHeapIndicesUAV.emplace_back();
				createUAV(handle, i);
			}
		}
	}
	uint32_t TextureD3D12::SubresourceIndexForMipLayer(uint32_t mip, uint32_t layer) const
	{
		return layer * numMips + mip;
	}
	TextureView TextureD3D12::GetDefaultView() const
	{
		return TextureView{ {
			.dsvIDX = dsvIDX,
			.rtvIDX = rtvIDX,
			.srvIDX = srvIDX,
			.uavIDX = uavIDX,
			.parentResource = this,
			.coveredMips = ALL_MIPS,
			.coveredLayers = ALL_LAYERS
		}};
	}
	TextureView TextureD3D12::GetViewForMip(uint32_t mip) const
	{

		bool hasSRV = mip < mipHeapIndicesSRV.size();
		bool hasUAV = mip < mipHeapIndicesUAV.size();
		bool hasRTV = mip < mipHeapIndicesRTV.size();

		return TextureView{ {
			.dsvIDX = dsvIDX,
			.rtvIDX = hasRTV ? mipHeapIndicesRTV.at(mip) : unallocated,
			.srvIDX = hasSRV ? mipHeapIndicesSRV.at(mip) : unallocated,
			.uavIDX = hasUAV ? mipHeapIndicesUAV.at(mip) : unallocated,
			.parentResource = this,
			.coveredMips = MakeMipMaskForIndex(mip),
			.coveredLayers = ALL_LAYERS,
		} };
	}
	RGLCustomTextureViewPtr TextureD3D12::MakeCustomTextureView(const CustomTextureViewConfig& config) const
	{
		return RGLCustomTextureViewPtr();
	}
	Dimension TextureD3D12::GetSize() const
	{
		return size;
	}
	TextureD3D12::~TextureD3D12()
	{
		texture = nullptr;
		if (allocation) {
			allocation->Release();
		}
		// release descriptors
		if (rtvAllocated()) {
			owningDevice->RTVHeap->DeallocateSingle(rtvIDX);
			rtvIDX = unallocated;
		}
		if (srvAllocated()) {
			owningDevice->CBV_SRV_UAVHeap->DeallocateSingle(srvIDX);
			srvIDX = unallocated;
		}
		if (dsvAllocated()) {
			owningDevice->DSVHeap->DeallocateSingle(dsvIDX);
			dsvIDX = unallocated;
		}
		if (uavAllocated()) {
			owningDevice->CBV_SRV_UAVHeap->DeallocateSingle(uavIDX);
		}
		for (const auto handle : mipHeapIndicesSRV) {
			owningDevice->CBV_SRV_UAVHeap->DeallocateSingle(handle);
		}
		for (const auto handle : mipHeapIndicesUAV) {
			owningDevice->CBV_SRV_UAVHeap->DeallocateSingle(handle);
		}
	}
	TextureView CustomTextureViewD3D12::GetView() const
	{
		return TextureView();
	}
}
#endif