#pragma once
#define NOMINMAX
#include <RGL/Types.hpp>
#include <RGL/Core.hpp>
#include <RGL/TextureFormat.hpp>
#include <RGL/Common.hpp>
#include "RGLCommon.hpp"
#include <cassert>
#include <comdef.h>
#include <wrl.h>
#include <format>
#include <d3d12.h>

inline void DX_CHECK(HRESULT dx_check_hr) {
	RGL::Assert(!FAILED(dx_check_hr), _com_error(dx_check_hr,nullptr).ErrorMessage());
}

namespace RGL {
	void InitD3D12(const RGL::InitOptions&);
	void DeintD3D12();
	struct RenderPassConfig;

	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> CreateDescriptorHeap(Microsoft::WRL::ComPtr<ID3D12Device2> device,
		D3D12_DESCRIPTOR_HEAP_TYPE type, uint32_t numDescriptors, D3D12_DESCRIPTOR_HEAP_FLAGS flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE);

	RGLRenderPassPtr CreateRenderPassD3D12(const RenderPassConfig& config);

	DXGI_FORMAT rgl2dxgiformat_texture(RGL::TextureFormat format);
	D3D12_RESOURCE_STATES rgl2d3d12resourcestate(RGL::ResourceLayout layout);
	D3D12_COMPARISON_FUNC rgl2d3dcompfn(RGL::DepthCompareFunction depthFunction);

	void RGLDeviceRemovedHandler(PVOID context, BOOLEAN);
}
