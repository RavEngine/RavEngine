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
			suffix = "_VS";
			break;
		case ShaderStage::Fragment:
			type = EShLangFragment;
			suffix = "_FS";
			break;
		case ShaderStage::TesseControl:
			type = EShLangTessControl;
			suffix = "_TC";
			break;
		case ShaderStage::TessEval:
			type = EShLangTessEvaluation;
			suffix = "_TE";
			break;
		case ShaderStage::Geometry:
			type = EShLangGeometry;
			suffix = "_G";
			break;
		case ShaderStage::Compute:
			type = EShLangCompute;
			suffix = "_C";
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
