#if RGL_DX12_AVAILABLE
#include "D3D12ShaderLibrary.hpp"
#include <d3dcompiler.h>
#include <d3d12shader.h>
#include <dxcapi.h>

#if __has_include(<librglc.hpp>)
#include <librglc.hpp>
#define RGL_CAN_RUNTIME_COMPILE 1
#endif
#include <atlbase.h>

extern CComPtr<IDxcUtils> dxcUtilsPtr;

namespace RGL {
	ShaderLibraryD3D12::ShaderLibraryD3D12()
	{
		FatalError("Not implemented");
	}
	ShaderLibraryD3D12::ShaderLibraryD3D12(const std::span<const uint8_t, std::dynamic_extent> bytes) : bufferBindingStore(std::make_shared<decltype(bufferBindingStore)::element_type>())
	{
		InitFromBytes(bytes);
	}
	ShaderLibraryD3D12::ShaderLibraryD3D12(const std::string_view source, const FromSourceConfig& config) : bufferBindingStore(std::make_shared<decltype(bufferBindingStore)::element_type>())
	{
#if RGL_CAN_RUNTIME_COMPILE
		auto result = librglc::CompileString(source, librglc::API::Direct3D12, static_cast<librglc::ShaderStage>(config.stage), {
			.outputBinary = true,
			.entrypointOutputName = "main",
		});
		InitFromBytes(std::span<uint8_t>{reinterpret_cast<uint8_t*>(result.data()), result.size()});
#else
		FatalError("RGL was not built with runtime shader compilation support");
#endif
	}
	ShaderLibraryD3D12::ShaderLibraryD3D12(const std::filesystem::path& path) : bufferBindingStore(std::make_shared<decltype(bufferBindingStore)::element_type>())
	{
		DX_CHECK(D3DReadFileToBlob(path.c_str(), &shaderBlob));
		auto size = shaderBlob->GetBufferSize();
		shaderBytecode = CD3DX12_SHADER_BYTECODE(shaderBlob.Get());
		ShaderReflect();
	}
	void ShaderLibraryD3D12::InitFromBytes(const std::span<const uint8_t, std::dynamic_extent> bytes)
	{
		D3DCreateBlob(bytes.size_bytes(), &shaderBlob);
		std::memcpy(shaderBlob->GetBufferPointer(), bytes.data(), bytes.size_bytes());
		shaderBytecode = CD3DX12_SHADER_BYTECODE(shaderBlob.Get());
		ShaderReflect();
	}
	void ShaderLibraryD3D12::ShaderReflect()
	{
#ifdef REFL_ENABLED
		DxcBuffer reflectionData{
			.Ptr = shaderBytecode.pShaderBytecode,
			.Size = shaderBytecode.BytecodeLength,
			.Encoding = DXC_CP_ACP,
		};
		CComPtr< ID3D12ShaderReflection > pReflection;
		dxcUtilsPtr->CreateReflection(&reflectionData, IID_PPV_ARGS(&pReflection));

		D3D12_SHADER_DESC desc;
		pReflection->GetDesc(&desc);

		for (UINT i = 0; i < desc.BoundResources; i++) {
			D3D12_SHADER_INPUT_BIND_DESC desc;
			pReflection->GetResourceBindingDesc(i,&desc);

			bool writable = false;
			bool isBuffer = false;
			switch (desc.Type) {
			case D3D_SIT_UAV_RWTYPED:
			case D3D_SIT_UAV_RWSTRUCTURED:
			case D3D_SIT_UAV_RWBYTEADDRESS:
			case D3D_SIT_UAV_APPEND_STRUCTURED:
			case D3D_SIT_UAV_CONSUME_STRUCTURED:
			case D3D_SIT_UAV_RWSTRUCTURED_WITH_COUNTER:
			case D3D_SIT_UAV_FEEDBACKTEXTURE:
				writable = true;
			case D3D_SIT_CBUFFER:
			case D3D_SIT_TBUFFER:
			case D3D_SIT_STRUCTURED:
			case D3D_SIT_BYTEADDRESS:
				isBuffer = true;
			default:
				break;
			}

			BufferBindingInfo info{
				.isUAV = writable
			};
			if (isBuffer) {
				bufferBindingStore->insert(std::make_pair(i, info));
			}
		}
#endif
	}
}

#endif