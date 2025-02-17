#pragma once
#include <string>
#include <vector>
#include <array>
#include <filesystem>


namespace spirv_cross{
struct Resource;
};

namespace shadert{

typedef std::vector<uint32_t> spirvbytes;

enum class ShaderStage{
	Vertex,
	Fragment,
	TessControl,
	TessEval,
	Geometry,
	Compute
};

enum class TargetAPI{
	OpenGL_ES = 0,
	OpenGL,
	Vulkan,
	HLSL,
	WGSL,
	DXIL,
	Metal,
#ifdef __APPLE__
	MetalBinary,	// requires xcrun 
#endif
};


struct FileCompileTask{
	const std::filesystem::path& filename;
	const ShaderStage stage;
	const std::vector<std::filesystem::path> includePaths;	// optional
};

struct MemoryCompileTask {
	const std::string source;
	const std::string sourceFileName;
	const ShaderStage stage;
	const std::vector<std::filesystem::path> includePaths;	// optional
};

struct ReflectData{
	struct Resource
	{
		uint32_t id;
		uint32_t type_id;
		uint32_t base_type_id;
		std::string name;
		Resource(const spirv_cross::Resource&);
	};
	std::vector<Resource> uniform_buffers;
	std::vector<Resource> storage_buffers;
	std::vector<Resource> stage_inputs;
	std::vector<Resource> stage_outputs;
	std::vector<Resource> subpass_inputs;
	std::vector<Resource> storage_images;
	std::vector<Resource> sampled_images;
	std::vector<Resource> atomic_counters;
	std::vector<Resource> acceleration_structures;
	
	// There can only be one push constant block,
	// but keep the vector in case this restriction is lifted in the future.
	std::vector<Resource> push_constant_buffers;
	
	// For Vulkan GLSL and HLSL source,
	// these correspond to separate texture2D and samplers respectively.
	std::vector<Resource> separate_images;
	std::vector<Resource> separate_samplers;
    
    std::array<uint16_t,3> compute_dim{};
};

struct Uniform
{
	std::string name;
	int glDefineType;
	uint8_t arraySize = 0;
	uint16_t bufferOffset = 0;
	uint8_t texComponent = 0;
	uint8_t texDimension = 0;
	uint16_t texFormat = 0;
};

struct LiveAttribute {
	std::string name;
};


struct IMResult{
	std::string sourceData, binaryData;
	ReflectData reflectData;
	std::vector<Uniform> uniformData;
	std::vector<LiveAttribute> attributeData;
};

struct CompileResult{
	IMResult data;
};

struct Options{
	uint32_t version;
	bool mobile;
	bool debug = false;
	bool enableInclude = true;
	std::string entryPoint = "frag";
	struct UniformBufferSettings{
		std::string newBufferName;
		bool renameBuffer = false;
	} uniformBufferSettings;
    
    struct BindlessSettings {
        uint32_t descSet = 0;
        bool deviceStorage = false;
        enum class Type : uint8_t{
            SampledImage,
            Buffer
        } type;
    };
    std::vector<BindlessSettings> mtlDeviceAddressSettings;
    
    struct PushConstantSettings{
        uint8_t firstIndex = 0;
        
    } pushConstantSettings;
    
    struct BufferBindingSettings{
        uint8_t stageInputSize = 0;
        
    } bufferBindingSettings;
    std::string preambleContent;   // Put defines here
};

class ShaderTranspiler{
public:
    /**
    Execute the shader transpiler using shader source code in a file.
     @param task the CompileTask to execute. See CompileTask for information.
     @param platform the target API to compile to.
     @return A CompileResult representing the result of the compile.
     */
	CompileResult CompileTo(const FileCompileTask& task, const TargetAPI platform, const Options& options);

	/**
	Execute the shader transpiler using shader source code in memory.
	 @param task the CompileTask to execute. See CompileTask for information.
	 @param platform the target API to compile to.
	 @return A CompileResult representing the result of the compile.
	 */
	CompileResult CompileTo(const MemoryCompileTask& task, const TargetAPI platform, const Options& options);

	~ShaderTranspiler();
};
}
