#pragma once
#include <string>
#include <string_view>
#include <vector>
#include <filesystem>
#include <cstdint>

namespace librglc {

    constexpr static auto MTL_FIRST_BUFFER = 6;

	enum class API : uint8_t {
		Vulkan,
		Direct3D12,
		Metal,
		WebGPU
	};

	enum class ShaderStage : uint8_t {
		Vertex,
		Fragment,
		Compute
	};

	struct Config {
		std::vector<std::filesystem::path> include_paths;
		std::vector<std::string> defines;
		bool outputBinary = false;
		bool enableDebug = false;
        std::string entrypointOutputName = "main";
	};

	std::string CompileString(const std::string_view source, API toAPI, ShaderStage input_stage, const Config& config);
	std::string CompileFile(const std::filesystem::path source, API toAPI, ShaderStage input_stage, const Config& config);
}
