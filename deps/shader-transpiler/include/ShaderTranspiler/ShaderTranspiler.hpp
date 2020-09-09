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
     * Factory
     * @return instance of DefaultTBuiltInResource struct with appropriate fields set
     */
    static TBuiltInResource CreateDefaultTBuiltInResource();
	
	/**
	 Compile GLSL to SPIR-V bytes
	 @param filename the file to compile
	 @param ShaderType the type of shader to compile
	 */
	const std::vector<uint32_t> CompileGLSL(const std::filesystem::path& filename, const EShLanguage ShaderType);
	
    /**
     Decompile SPIR-V to OpenGL ES shader
     @param bin the SPIR-V binary to decompile
     @return OpenGL-ES source code
     */
	std::string SPIRVToESSL(const spirvbytes& bin);
    
    /**
     Decompile SPIR-V to DirectX shader
     @param bin the SPIR-V binary to decompile
     @return HLSL source code
     */
	std::string SPIRVToHLSL(const spirvbytes& bin);
    
    /**
     Decompile SPIR-V to Metal shader
     @param bin the SPIR-V binary to decompile
     @param mobile set to True to compile for Apple Mobile platforms
     @return Metal shader source code
     */
	std::string SPIRVtoMSL(const spirvbytes& bin, bool mobile = false);
public:
    /**
    Execute the shader transpiler.
     @param task the CompileTask to execute. See CompileTask for information.
     @param platform the target API to compile to.
     @return A CompileResult representing the result of the compile.
     */
	CompileResult CompileTo(const CompileTask& task, TargetAPI platform);
};
