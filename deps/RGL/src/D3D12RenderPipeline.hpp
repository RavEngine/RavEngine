#pragma once
#define NOMINMAX
#include <RGL/Types.hpp>
#include <RGL/Pipeline.hpp>
#include "RGLD3D12.hpp"
#include <directx/d3d12.h>
#include <directx/d3dx12.h>
#include <vector>
#include <unordered_map>
#include "D3D12PipelineShared.hpp"

namespace RGL {

	struct textureBindingKey {
		uint32_t binding;
		uint32_t space = 0;
		textureBindingKey(uint32_t binding) : binding(binding) {}
		textureBindingKey(uint32_t binding, uint32_t space) : binding(binding), space(space) {}

		bool operator==(const textureBindingKey& other) const {
			return binding == other.binding && space == other.space;
		}
	};
}

namespace std {
	template <> struct hash<RGL::textureBindingKey>
	{
		size_t operator()(const RGL::textureBindingKey& x) const
		{
			return size_t(x.binding) << 32 + x.space;
		}
	};
}


namespace RGL {
	struct DeviceD3D12;
	struct PipelineLayoutD3D12 : public IPipelineLayout {
		const std::shared_ptr<DeviceD3D12> owningDevice;
		Microsoft::WRL::ComPtr<ID3D12RootSignature> rootSignature;
		const PipelineLayoutDescriptor config;

		struct bufferBindInfo {
			uint32_t slot;
			bool isUAV;
		};

		struct textureBindInfo {
			uint32_t slot;
			bool isUAV;
		};

		

		std::unordered_map<uint32_t, bufferBindInfo> bufferBindingToRootSlot;
		std::unordered_map<textureBindingKey, textureBindInfo> textureBindingToRootSlot;
		std::unordered_map < uint32_t, uint32_t> samplerBindingtoRootSlot;

		auto slotForBufferIdx(uint32_t bindingPos) {
			return bufferBindingToRootSlot.at(bindingPos).slot;
		}

		auto slotForSamplerIdx(uint32_t bindingPos) {
			return samplerBindingtoRootSlot.at(bindingPos);
		}

		auto slotForTextureIdx(uint32_t bindingPos, bool isBindless) {
			return textureBindingToRootSlot.at({ bindingPos, isBindless? bindingPos : 0u });
		}

		bool bufferIdxIsUAV(uint32_t bindingPos) {
			return bufferBindingToRootSlot.at(bindingPos).isUAV;
		}

		PipelineLayoutD3D12(decltype(owningDevice), const PipelineLayoutDescriptor&);
	};

	struct RenderPipelineD3D12 : public IRenderPipeline {
		const std::shared_ptr<DeviceD3D12> owningDevice;
		Microsoft::WRL::ComPtr<ID3D12PipelineState> pipelineState;
		const std::shared_ptr<PipelineLayoutD3D12> pipelineLayout;
		const D3D12_PRIMITIVE_TOPOLOGY overrideMode;

		BufferBindingStore vsBufferBindings, fsBufferBindings;

		RenderPipelineD3D12(decltype(owningDevice), const RenderPipelineDescriptor&);
	};
}
