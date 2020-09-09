#pragma once
#include <string>
#include <vector>
#include <filesystem>
#include <glslang/Public/ShaderLang.h>

enum class ShaderStage{
	Vertex,
	Fragment,
	TesseControl,
	TessEval,
	Geometry,
	Compute
};

enum class TargetAPI{
	OpenGL_ES = 0,
	OpenGL,
	Vulkan,
	DirectX11,
	Metal_Mac,
	Metal_Mobile,
};

struct CompileTask{
	const std::filesystem::path& filename;
	ShaderStage stage;
};

struct CompileResult{
	std::string data;
	bool isBinary;
	std::string suffix;
};

class ShaderTranspiler{
protected:
	typedef std::vector<uint32_t> spirvbytes;
	static bool glslAngInitialized;
	
	/**
	 Compile GLSL to SPIR-V bytes
	 @param filename the file to compile
	 @param ShaderType the type of shader to compile
	 */
	const std::vector<uint32_t> CompileGLSL(const std::filesystem::path& filename, const EShLanguage ShaderType);
	
	std::string SPIRVToESSL(const spirvbytes& bin);
	std::string SPIRVToHLSL(const spirvbytes& bin);
	std::string SPIRVtoMSL(const spirvbytes& bin, bool mobile = false);
	
	//see https://github.com/ForestCSharp/VkCppRenderer/blob/master/Src/Renderer/GLSL/ShaderCompiler.hpp for options
	const TBuiltInResource DefaultTBuiltInResource{
		.maxDrawBuffers = 32
	};
public:
	CompileResult CompileTo(const CompileTask& task, TargetAPI platform);
};
