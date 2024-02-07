#include "librglc.hpp"
#include <ShaderTranspiler/ShaderTranspiler.hpp>
#include <stdexcept>

using namespace shadert;

shadert::ShaderStage rgl2shadert(librglc::ShaderStage stage) {
	switch (stage) {
	case decltype(stage)::Vertex: return shadert::ShaderStage::Vertex;
	case decltype(stage)::Fragment: return shadert::ShaderStage::Fragment;
	case decltype(stage)::Compute: return shadert::ShaderStage::Compute;
	default:
		throw std::runtime_error("Not a valid stage");
	}
}

shadert::TargetAPI rgl2shadert_source(librglc::API api) {
	switch (api) {
	case decltype(api)::Direct3D12: return TargetAPI::HLSL;
	case decltype(api)::Metal: return TargetAPI::Metal;
	case decltype(api)::WebGPU: return TargetAPI::WGSL;
	default:
		throw std::runtime_error("Cannot create a source target for API");
	}
}

shadert::TargetAPI rgl2shadert_binary(librglc::API api) {
	switch (api) {
#if _WIN32
	case decltype(api)::Direct3D12: return TargetAPI::DXIL;
#endif
	case decltype(api)::Vulkan: return TargetAPI::Vulkan;
#ifdef __APPLE__
	case decltype(api)::Metal: return TargetAPI::MetalBinary;
#endif
	case decltype(api)::WebGPU: return TargetAPI::WGSL;
	default:
		throw std::runtime_error("Cannot create a binary target for API");
	}
}

namespace librglc {
	auto CompileAny(auto task, API toAPI, ShaderStage input_stage, const Config& config) {
		shadert::Options opt;
		opt.mobile = false;
		opt.debug = config.enableDebug;
		opt.entryPoint = config.entrypointOutputName;
		if (toAPI == API::Vulkan) {
			opt.version = 15;
		}
		else if (toAPI == API::Direct3D12) {
			opt.version = 64;
		}
		else if (toAPI == API::Metal) {
			opt.version = 30;
            opt.pushConstantSettings.firstIndex = MTL_FIRST_BUFFER;    // the [[stage_input]] consumes slot 0, extra vertex buffers consume the next slots
		}
		else if (toAPI == API::WebGPU) {
			opt.version = 13;
		}

		ShaderTranspiler s;
		auto result = s.CompileTo(task, config.outputBinary ? rgl2shadert_binary(toAPI) : rgl2shadert_source(toAPI), opt);

		return config.outputBinary ? result.data.binaryData : result.data.sourceData;
	}

	std::string CompileString(const std::string_view source, API toAPI, ShaderStage input_stage, const Config& config)
	{
		MemoryCompileTask task{
			.source = std::string(source),
			.stage = rgl2shadert(input_stage),
			.includePaths = config.include_paths,
		};
		return CompileAny(task,toAPI,input_stage,config);
	}
	
	std::string CompileFile(const std::filesystem::path source, API toAPI, ShaderStage input_stage, const Config& config)
	{
		FileCompileTask task{
			.filename = source,
			.stage = rgl2shadert(input_stage),
			.includePaths = config.include_paths,
		};
		return CompileAny(task, toAPI, input_stage, config);
	}

}
