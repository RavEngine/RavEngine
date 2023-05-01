#if RGL_DX12_AVAILABLE
#include "D3D12ShaderLibrary.hpp"
#include <d3dcompiler.h>
#include <librglc.hpp>

namespace RGL {
	ShaderLibraryD3D12::ShaderLibraryD3D12()
	{
		FatalError("Not implemented");
	}
	ShaderLibraryD3D12::ShaderLibraryD3D12(const std::span<uint8_t, std::dynamic_extent> bytes)
	{
		InitFromBytes(bytes);
	}
	ShaderLibraryD3D12::ShaderLibraryD3D12(const std::string_view source, const FromSourceConfig& config)
	{
		auto result = librglc::CompileString(source, librglc::API::Direct3D12, static_cast<librglc::ShaderStage>(config.stage), {
			.outputBinary = true,
			.entrypointOutputName = "main",
		});
		InitFromBytes(std::span<uint8_t>{reinterpret_cast<uint8_t*>(result.data()), result.size()});
	}
	ShaderLibraryD3D12::ShaderLibraryD3D12(const std::filesystem::path& path)
	{
		DX_CHECK(D3DReadFileToBlob(path.c_str(), &shaderBlob));
		shaderBytecode = CD3DX12_SHADER_BYTECODE(shaderBlob.Get());
	}
	void ShaderLibraryD3D12::InitFromBytes(const std::span<uint8_t, std::dynamic_extent> bytes)
	{
		D3DCreateBlob(bytes.size_bytes(), &shaderBlob);
		std::memcpy(shaderBlob->GetBufferPointer(), bytes.data(), bytes.size_bytes());
		shaderBytecode = CD3DX12_SHADER_BYTECODE(shaderBlob.Get());
	}
}

#endif