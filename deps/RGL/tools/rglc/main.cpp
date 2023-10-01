#include <librglc.hpp>
#include <cxxopts.hpp>
#include <filesystem>
#include <iostream>
#include <stdexcept>
#include <fstream>

#ifdef NDEBUG
#define CATCH_ERRORS 1
#else
#define CATCH_ERRORS 1
#endif

#define FATAL(reason) {std::cerr << "rglc error: " << reason << std::endl; return 1;}

using namespace std;

int main(int argc, const char** argv) {
	cxxopts::Options options("rglc", "RGL Shader Compiler");
	options.add_options()
		("d,debug", "Enable debugging") // a bool parameter
		("v,version", "Print version information")
		("f,file", "Input file path", cxxopts::value<filesystem::path>())
		("o,output", "Ouptut file path", cxxopts::value<filesystem::path>())
        ("b,binary", "Create a binary shader")
        ("e,entrypoint", "Set the name of the generated entrypoint, defaults to \"main\"", cxxopts::value<std::string>())
		("a,api", "Target API", cxxopts::value<string>())
		("s,stage", "Shader stage", cxxopts::value<std::string>())
		("i,include", "Include paths", cxxopts::value<std::vector<filesystem::path>>())
		("h,help", "Show help menu")
		;
	auto args = options.parse(argc, argv);
	if (args["version"].as<bool>()) {
		printf("rglc version 0.0.1\n");
		return 0;
	}
	if (args["help"].as<bool>()) {
		cout << options.help() << endl;
		return 0;
	}
	
	bool debug = false;
	try {
		debug = args["debug"].as<decltype(debug)>();
	}
	catch (exception& e) {}
    
    std::string entrypoint = "main";
    try{
        entrypoint = args["entrypoint"].as<decltype(entrypoint)>();
    }
    catch(exception& e){}


	// check for input and output file
	std::filesystem::path inputFile;
	try {
		inputFile = std::move(args["file"].as<decltype(inputFile)>());
	}
	catch (exception& e) {
		FATAL("no input file")
	}
	std::filesystem::path outputFile;
	try {
		outputFile = std::move(args["output"].as<decltype(outputFile)>());
	}
	catch (exception& e) {
		FATAL("no output file")
	}

	// get the stage
	librglc::ShaderStage inputStage;
	try {
		auto& shaderStageString = args["stage"].as<string>();
		static const unordered_map<string, decltype(inputStage)> shaderStageMap{
			{"vertex", decltype(inputStage)::Vertex},
			{"fragment",decltype(inputStage)::Fragment},
			{"compute", decltype(inputStage)::Compute}
		};
		try {
			inputStage = shaderStageMap.at(shaderStageString);
		}
		catch (exception& e) {
			fprintf(stderr, "rglc error: %s is not a valid shader stage\nExpected one of\n", shaderStageString.c_str());
			for (const auto& row : shaderStageMap) {
				cerr << "\t" << row.first << "\n";
			}
			return 1;
		}
	}
	catch (exception& e) {
		FATAL("shader stage not provided")
	}

	// get include directories
	std::vector<filesystem::path> includepaths;
	try {
		includepaths = args["include"].as<decltype(includepaths)>();
	}
	catch (exception& e) {
		//FATAL("could not load include paths");
	}

	// get the target API
	librglc::API api;
	const char* entryPoint = nullptr;
	//shadert::Options opt;
	try {
		auto& apiString = args["api"].as<string>();
		struct apiData {
			decltype(api) e_api;
		};
		static const unordered_map<string, apiData> apiMap{
			{"Vulkan", {decltype(api)::Vulkan}},
			{"Metal", {decltype(api)::Metal}},
			{"Direct3D12", {decltype(api)::Direct3D12}},
			{"WebGPU", {decltype(api)::WebGPU}}
		};
		try {
			auto& data = apiMap.at(apiString);
			api = data.e_api;
			//opt = std::move(data.options);
		}
		catch (exception& e) {
			fprintf(stderr, "rglc error: %s is not a valid API\n Expected one of:\n", apiString.c_str());
			for (const auto& row : apiMap) {
				cerr << "\t" << row.first << "\n";
			}
			return 1;
		}
	}
	catch (exception& e) {
		FATAL("target API not provided")
	}
    
    // targets that only allow binary output ignore the binary flag
    bool binary = (api == librglc::API::Vulkan);
    if (!binary){
        try{
            binary = args["binary"].as<decltype(binary)>();
        }
        catch(exception& e){}
    }
   

#if CATCH_ERRORS
	try 
#endif
	{
		auto result = librglc::CompileFile(inputFile, api, inputStage, { .include_paths = includepaths, .outputBinary = binary, .enableDebug = debug, .entrypointOutputName = entrypoint });
		std::filesystem::create_directories(outputFile.parent_path());		// make all the folders necessary
		ofstream out(outputFile, ios::out | ios::binary);
		out.write(result.data(), result.size() * sizeof(decltype(result)::value_type));
		if (!out.good()) {
			fprintf(stderr, "Error writing to %s", outputFile.string().c_str());
			return 1;
		}
	}
#if CATCH_ERRORS
	catch (exception& e) {
		FATAL(e.what());
	}
#endif

	return 0;
	
}
