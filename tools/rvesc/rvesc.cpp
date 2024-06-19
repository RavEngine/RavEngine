#include <filesystem>
#include <simdjson.h>
#include <cxxopts.hpp>
#include <librglc.hpp>
#include <fstream>
#include <cmrc/cmrc.hpp>
#include <fmt/format.h>

CMRC_DECLARE(rvesc_resources);

using namespace std;


#define FATAL(reason) {std::cerr << "rvesc error: " << reason << std::endl; return 1;}

int do_compile(const std::filesystem::path& in_desc_file, const std::filesystem::path& outfile, const std::vector<std::filesystem::path>& includeDirs, librglc::API targetAPI, bool debug) {
	simdjson::ondemand::parser parser;

	auto json = simdjson::padded_string::load(in_desc_file.string());
	simdjson::ondemand::document doc = parser.iterate(json);

	const auto json_dir = in_desc_file.parent_path();

	auto infile = json_dir / std::string_view(doc["shader"]);
	std::string_view mat_type = doc["type"];

	std::string_view shaderStageString = doc["stage"];

	librglc::ShaderStage inputStage;
	try {
		static const unordered_map<string_view, decltype(inputStage)> shaderStageMap{
			{"vertex", decltype(inputStage)::Vertex},
			{"fragment",decltype(inputStage)::Fragment},
			{"compute", decltype(inputStage)::Compute}
		};
		try {
			inputStage = shaderStageMap.at(shaderStageString);
		}
		catch (exception& e) {
			fprintf(stderr, "rglc error: %s is not a valid shader stage\nExpected one of\n", shaderStageString.data());
			for (const auto& row : shaderStageMap) {
				cerr << "\t" << row.first << "\n";
			}
			return 1;
		}
	}
	catch (exception& e) {
		FATAL("shader stage not provided")
	}

	std::string entrypoint = "main";
	if (targetAPI == librglc::API::Metal) {
		entrypoint = outfile.filename().string();
        entrypoint = entrypoint.substr(0,entrypoint.find_last_of('.'));
		std::replace(entrypoint.begin(), entrypoint.end(), '.', '_');
	}


	std::string shaderTemplateName;

	if (mat_type == "lit-mesh") {
		if (inputStage == librglc::ShaderStage::Vertex) {
			shaderTemplateName = "lit_mesh.vsh";
		}
		else {
			shaderTemplateName = "lit_mesh.fsh";
		}
	}
	else if (mat_type == "skybox") {
		if (inputStage == librglc::ShaderStage::Vertex) {
			shaderTemplateName = "skybox.vsh";
		}
		else {
			shaderTemplateName = "skybox.fsh";
		}
	}
	else if (mat_type == "unlit-mesh") {
		if (inputStage == librglc::ShaderStage::Vertex) {
			shaderTemplateName = "unlit_mesh.vsh";
		}
		else {
			shaderTemplateName = "unlit_mesh.fsh";
		}
	}
	else if (mat_type == "lit-particle-quad") {
		if (inputStage == librglc::ShaderStage::Vertex) {
			shaderTemplateName = "particle_quad.vsh";
		}
		else {
			shaderTemplateName = "lit_particle_quad.fsh";
		}
	}
	else if (mat_type == "particle-update") {
		inputStage = librglc::ShaderStage::Compute;
		shaderTemplateName = "particle_update.csh";
	}
	else if (mat_type == "particle-init") {
		inputStage = librglc::ShaderStage::Compute;
		shaderTemplateName = "particle_init.csh";
	}
	else {
		FATAL(fmt::format("{} is not a supported material type",mat_type));
		return 1;
	}

	auto resources = cmrc::rvesc_resources::get_filesystem();
	assert(resources.exists(shaderTemplateName));
	auto templateFile = resources.open(shaderTemplateName);

	std::string shaderTemplate{ templateFile.begin(), templateFile.end() };

	auto name = std::filesystem::relative(infile, in_desc_file.parent_path());
	//const auto normalized = name.lexically_normal().string();


	std::vector<char> buffer(shaderTemplate.size() + infile.string().length(), 0);
	snprintf(buffer.data(), buffer.size(), shaderTemplate.data(), infile.string().c_str());
	std::string full_shader = buffer.data();

	// get the defines
	simdjson::ondemand::array array;
	auto err = doc["defines"].get(array);
	std::vector<std::string> defines;
	if (err) {
		// no defines were present
	}
	else {
		// pull out the defines
		for (auto str : array) {
			defines.push_back(std::string(str.get_string().value()));
		}
	}


	try {
		auto result = librglc::CompileString(full_shader, targetAPI, inputStage, {
			.include_paths = includeDirs, 
			.defines = defines,
			.outputBinary = targetAPI == librglc::API::Vulkan, 
			.enableDebug = debug, 
			.entrypointOutputName = entrypoint
		});

		std::filesystem::create_directories(outfile.parent_path());		// make all the folders necessary
		ofstream out(outfile, ios::out | ios::binary);
		out.write(result.data(), result.size() * sizeof(decltype(result)::value_type));
		if (!out.good()) {
			FATAL(fmt::format("Error writing to {}\n", outfile.string()));
		}
	}
	catch (exception& e) {
		FATAL(e.what());
	}

	return 0;
}



int main(int argc, char** argv) {
	cxxopts::Options options("rglc", "RGL Shader Compiler");
	options.add_options()
		("d,debug", "Enable debugging") // a bool parameter
		("f,file", "Input file path", cxxopts::value<filesystem::path>())
		("o,output", "Ouptut file path", cxxopts::value<filesystem::path>())
		("a,api", "Target API", cxxopts::value<string>())
		("s,stage", "Shader stage", cxxopts::value<std::string>())
		("i,include", "Include paths", cxxopts::value<std::vector<filesystem::path>>())
		("h,help", "Show help menu")
		;

	auto args = options.parse(argc, argv);

	if (args["help"].as<bool>()) {
		cout << options.help() << endl;
		return 0;
	}

	bool debug = false;
	try {
		debug = args["debug"].as<decltype(debug)>();
	}
	catch (exception& e) {}

	std::filesystem::path inputFile;
	try {
		inputFile = args["file"].as<decltype(inputFile)>();
	}
	catch (exception& e) {
		FATAL("no input file")
	}
	std::filesystem::path outputFile;
	try {
		outputFile = args["output"].as<decltype(outputFile)>();
	}
	catch (exception& e) {
		FATAL("no output file")
	}

	// get include directories
	std::vector<filesystem::path> includepaths;
	try {
		includepaths = args["include"].as<decltype(includepaths)>();
	}
	catch (exception& e) {
	}

	includepaths.push_back(std::filesystem::current_path());

	// get the target API
	librglc::API api;
	const char* entryPoint = nullptr;
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


	return do_compile(inputFile, outputFile, includepaths, api, debug);;
}
