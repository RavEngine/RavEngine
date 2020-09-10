#include <ShaderTranspiler.hpp>
#include <SPIRV/GlslangToSpv.h>
#include <StandAlone/DirStackFileIncluder.h>
#include <filesystem>
#include <spirv_glsl.hpp>
#include <spirv_hlsl.hpp>
#include <spirv_msl.hpp>


using namespace std;
using namespace std::filesystem;

bool ShaderTranspiler::glslAngInitialized = false;

//see https://github.com/ForestCSharp/VkCppRenderer/blob/master/Src/Renderer/GLSL/ShaderCompiler.hpp for more info
TBuiltInResource ShaderTranspiler::CreateDefaultTBuiltInResource(){
    return {
        .maxLights = 32,
        .maxClipPlanes = 6,
        .maxTextureUnits =  32,
        .maxTextureCoords =  32,
        .maxVertexAttribs =  64,
        .maxVertexUniformComponents =  4096,
        .maxVaryingFloats =  64,
        .maxVertexTextureImageUnits =  32,
        .maxCombinedTextureImageUnits =  80,
        .maxTextureImageUnits =  32,
        .maxFragmentUniformComponents =  4096,
        .maxDrawBuffers =  32,
        .maxVertexUniformVectors =  128,
        .maxVaryingVectors =  8,
        .maxFragmentUniformVectors =  16,
         .maxVertexOutputVectors =  16,
         .maxFragmentInputVectors =  15,
         .minProgramTexelOffset =  -8,
         .maxProgramTexelOffset =  7,
         .maxClipDistances =  8,
         .maxComputeWorkGroupCountX =  65535,
         .maxComputeWorkGroupCountY =  65535,
         .maxComputeWorkGroupCountZ =  65535,
         .maxComputeWorkGroupSizeX =  1024,
         .maxComputeWorkGroupSizeY =  1024,
         .maxComputeWorkGroupSizeZ =  64,
         .maxComputeUniformComponents =  1024,
         .maxComputeTextureImageUnits =  16,
         .maxComputeImageUniforms =  8,
         .maxComputeAtomicCounters =  8,
         .maxComputeAtomicCounterBuffers =  1,
         .maxVaryingComponents =  60,
         .maxVertexOutputComponents =  64,
         .maxGeometryInputComponents =  64,
         .maxGeometryOutputComponents =  128,
         .maxFragmentInputComponents =  128,
         .maxImageUnits =  8,
         .maxCombinedImageUnitsAndFragmentOutputs =  8,
         .maxCombinedShaderOutputResources =  8,
         .maxImageSamples =  0,
         .maxVertexImageUniforms =  0,
         .maxTessControlImageUniforms =  0,
         .maxTessEvaluationImageUniforms =  0,
         .maxGeometryImageUniforms =  0,
         .maxFragmentImageUniforms =  8,
         .maxCombinedImageUniforms =  8,
         .maxGeometryTextureImageUnits =  16,
         .maxGeometryOutputVertices =  256,
         .maxGeometryTotalOutputComponents =  1024,
         .maxGeometryUniformComponents =  1024,
         .maxGeometryVaryingComponents =  64,
         .maxTessControlInputComponents =  128,
         .maxTessControlOutputComponents =  128,
         .maxTessControlTextureImageUnits =  16,
         .maxTessControlUniformComponents =  1024,
         .maxTessControlTotalOutputComponents =  4096,
         .maxTessEvaluationInputComponents =  128,
         .maxTessEvaluationOutputComponents =  128,
         .maxTessEvaluationTextureImageUnits =  16,
         .maxTessEvaluationUniformComponents =  1024,
         .maxTessPatchComponents =  120,
         .maxPatchVertices =  32,
         .maxTessGenLevel =  64,
         .maxViewports =  16,
         .maxVertexAtomicCounters =  0,
         .maxTessControlAtomicCounters =  0,
         .maxTessEvaluationAtomicCounters =  0,
         .maxGeometryAtomicCounters =  0,
         .maxFragmentAtomicCounters =  8,
         .maxCombinedAtomicCounters =  8,
         .maxAtomicCounterBindings =  1,
         .maxVertexAtomicCounterBuffers =  0,
         .maxTessControlAtomicCounterBuffers =  0,
         .maxTessEvaluationAtomicCounterBuffers =  0,
         .maxGeometryAtomicCounterBuffers =  0,
         .maxFragmentAtomicCounterBuffers =  1,
         .maxCombinedAtomicCounterBuffers =  1,
         .maxAtomicCounterBufferSize =  16384,
         .maxTransformFeedbackBuffers =  4,
         .maxTransformFeedbackInterleavedComponents =  64,
         .maxCullDistances =  8,
         .maxCombinedClipAndCullDistances =  8,
         .maxSamples =  4,
         .limits =  {
             .nonInductiveForLoops =  1,
             .whileLoops =  1,
             .doWhileLoops =  1,
             .generalUniformIndexing =  1,
             .generalAttributeMatrixVectorIndexing =  1,
             .generalVaryingIndexing =  1,
             .generalSamplerIndexing =  1,
             .generalVariableIndexing =  1,
             .generalConstantMatrixVectorIndexing =  1,
        }
    };
}

const vector<uint32_t> ShaderTranspiler::CompileGLSL(const std::filesystem::path& filename, const EShLanguage ShaderType){
	//initialize. Do only once per process!
	if (!glslAngInitialized)
	{
		glslang::InitializeProcess();
		glslAngInitialized = true;
	}
	
	//Load GLSL into a string
	std::ifstream file(filename);
	
	if (!file.is_open())
	{
		throw std::runtime_error("failed to open file: " + filename.string());
	}
		
	//read input file into string, convert to C string
	std::string InputGLSL((std::istreambuf_iterator<char>(file)),std::istreambuf_iterator<char>());
	const char* InputCString = InputGLSL.c_str();
	
	//determine the stage
	glslang::TShader Shader(ShaderType);
	
	//set the associated strings (in this case one, but shader meta JSON can describe more. Pass as a C array and a size.
	Shader.setStrings(&InputCString, 1);
	
	//=========== vulkan versioning (should alow this to be passed in, or find out from the system) ========
	const int DefaultVersion = 130;
	
	int ClientInputSemanticsVersion = DefaultVersion; // maps to, say, #define VULKAN 100
	glslang::EShTargetClientVersion VulkanClientVersion = glslang::EShTargetVulkan_1_2;
	glslang::EShTargetLanguageVersion TargetVersion = glslang::EShTargetSpv_1_5;
	
	Shader.setEnvInput(glslang::EShSourceGlsl, ShaderType, glslang::EShClientVulkan, ClientInputSemanticsVersion);
	Shader.setEnvClient(glslang::EShClientVulkan, VulkanClientVersion);
	Shader.setEnvTarget(glslang::EShTargetSpv, TargetVersion);
	
    auto DefaultTBuiltInResource = CreateDefaultTBuiltInResource();
    
	TBuiltInResource Resources(DefaultTBuiltInResource);
	EShMessages messages = (EShMessages) (EShMsgSpvRules | EShMsgVulkanRules);
	
	
	
	// =============================== preprocess GLSL =============================
	DirStackFileIncluder Includer;
	
	//Get Path of File
	std::string Path = path(filename).stem();
	Includer.pushExternalLocalDirectory(Path);
	
	std::string PreprocessedGLSL;
	
	if (!Shader.preprocess(&Resources, DefaultVersion, ENoProfile, false, false, messages, &PreprocessedGLSL, Includer))
	{
		string msg = "GLSL Preprocessing Failed for: " + filename.string() + "\n" + Shader.getInfoLog() + "\n" + Shader.getInfoDebugLog();
		throw std::runtime_error(msg);
	}
	
	// update the stored strings (is the original set necessary?)
	const char* PreprocessedCStr = PreprocessedGLSL.c_str();
	Shader.setStrings(&PreprocessedCStr, 1);
	
	// ================ now parse the shader ================
	if (!Shader.parse(&Resources, DefaultVersion, false, messages))
	{
		string msg = "GLSL Parsing Failed for: " + filename.string() + "\n" + Shader.getInfoLog() + "\n" + Shader.getInfoDebugLog();
		throw std::runtime_error(msg);
	}
	
	// ============== pass parsed shader and link it ==============
	glslang::TProgram Program;
	Program.addShader(&Shader);
	
	if(!Program.link(messages))
	{
		std::string msg = "GLSL Linking Failed for: " + filename.string() + "\n" + Shader.getInfoLog() + "\n" + Shader.getInfoDebugLog();
		throw std::runtime_error(msg);
	}
	
	// ========= convert to spir-v =============
	std::vector<unsigned int> SpirV;
	spv::SpvBuildLogger logger;
	glslang::SpvOptions spvOptions;
	glslang::GlslangToSpv(*Program.getIntermediate(ShaderType), SpirV, &logger, &spvOptions);
	
	return SpirV;
}

std::string ShaderTranspiler::SPIRVToESSL(const spirvbytes& bin){
	spirv_cross::CompilerGLSL glsl(std::move(bin));
		
	//set options
	spirv_cross::CompilerGLSL::Options options;
	options.version = 330;
	options.es = true;
	glsl.set_common_options(options);
	
	return glsl.compile();
}

std::string ShaderTranspiler::SPIRVToHLSL(const spirvbytes& bin){
	spirv_cross::CompilerHLSL hlsl(std::move(bin));
	
	spirv_cross::CompilerHLSL::Options options;
	hlsl.set_hlsl_options(options);
	
	return hlsl.compile();
}

std::string ShaderTranspiler::SPIRVtoMSL(const spirvbytes& bin, bool mobile){
	spirv_cross::CompilerMSL msl(std::move(bin));
	
	spirv_cross::CompilerMSL::Options options;
	msl.set_msl_options(options);
	return msl.compile();
}

CompileResult ShaderTranspiler::CompileTo(const CompileTask& task, TargetAPI api){
	EShLanguage type;
	string suffix;
	switch(task.stage){
		case ShaderStage::Vertex:
			type = EShLangVertex;
			suffix = "VS";
			break;
		case ShaderStage::Fragment:
			type = EShLangFragment;
			suffix = "FS";
			break;
		case ShaderStage::TesseControl:
			type = EShLangTessControl;
			suffix = "TC";
			break;
		case ShaderStage::TessEval:
			type = EShLangTessEvaluation;
			suffix = "TE";
			break;
		case ShaderStage::Geometry:
			type = EShLangGeometry;
			suffix = "G";
			break;
		case ShaderStage::Compute:
			type = EShLangCompute;
			suffix = "C";
			break;
	}
	
	//generate spirv
	auto spirv = CompileGLSL(task.filename, type);
	switch(api){
		case TargetAPI::OpenGL_ES:
			return CompileResult{SPIRVToESSL(spirv),false,suffix + ".gles"};
		case TargetAPI::OpenGL:
			break;
		case TargetAPI::Vulkan:
			//optimize spirv further
			break;
		case TargetAPI::DirectX11:
			return CompileResult{SPIRVToHLSL(spirv),false, suffix + ".hlsl"};
			break;
		case TargetAPI::Metal_Mac:
		case TargetAPI::Metal_Mobile:
			return CompileResult{SPIRVtoMSL(spirv,api == TargetAPI::Metal_Mobile),false, suffix + ".metal"};
			break;
	}
}
