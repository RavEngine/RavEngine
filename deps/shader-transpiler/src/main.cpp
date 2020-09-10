//#include <ShaderConductor/ShaderConductor.hpp>
#include <json.hpp>
#include <cxxopts.hpp>
#include <fstream>
#include <cassert>
#include <iostream>
#include "ShaderTranspiler.hpp"

using namespace std;
using namespace std::filesystem;
using namespace nlohmann;

enum class TargetLang{
	HLSL,
	MSL,
	SPIRV
};



enum class TargetPlatform{
	macOS,
	Windows,
	Linux,
	iOS
};

//map input strings to stages
const unordered_map<string, ShaderStage> stagemap{
	{"vertex",ShaderStage::Vertex},
	{"fragment",ShaderStage::Fragment},
	{"geometry",ShaderStage::Geometry},
	{"tesseval",ShaderStage::TessEval},
	{"tesscontrol",ShaderStage::TesseControl},
	{"compute",ShaderStage::Compute},

};

int main(int argc, const char** argv){
	
	//get command line arguments
	try{
		cxxopts::Options args("shader-transpiler","Converts shader metadata files to appropriate deployment shaders");
		args.add_options()
			("f,file","Input shader metadata source file",cxxopts::value<string>())
			("o,output","output destination directory",cxxopts::value<string>());
		auto arguments = args.parse(argc, argv);
		
		ShaderTranspiler s;
		
		auto name = arguments["file"].as<std::string>();
		path outdir(arguments["output"].as<std::string>());
		
		//read files
		string metadata_src;
		{
			ifstream in(name);
			if (!in.good()){
				cerr << "ERROR: '" << name << "' not found." << endl;
				return 1;
			}
			string m((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
			metadata_src = m;
		}
			
		//compile each version
		auto j = json::parse(metadata_src);
		for (auto& definition : j){
			path leaf(definition["file"]);
			path file = path(name).parent_path() / leaf;
			auto stage = stagemap.at(string(definition["stage"]));
			
			auto task = CompileTask{file,stage};
			
#ifdef __APPLE__
			TargetAPI target = TargetAPI::Metal_Mac;
#elif defined _WIN32
			TargetAPI target = TargetAPI::DirectX11;
#elif defined __linux__
			TargetAPI target = TargetAPI::Vulkan;
#else
#error Platform not supported.
#endif
			auto result = s.CompileTo(task,target);
			
			string finalpath = leaf.string().substr(0,leaf.string().size() - (leaf.extension().string().size())) + result.suffix;
			
			ofstream output(outdir / finalpath);
			if (output.good()){
				output << result.data;
			}
			else{
				throw runtime_error("Could not open '" + finalpath + "' for writing.");
			}
		}
		return 0;
	}
	catch(exception& err){
		cerr << err.what() << endl;
		return 1;
	}
	
}
