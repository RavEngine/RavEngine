#include <iostream>
#include <fstream>
#include <cxxopts.hpp>
#include <nlohmann/json.hpp>
#include <filesystem>

using namespace std;
using namespace nlohmann;
using namespace std::filesystem;

int main(int argc, char** argv){
	cxxopts::Options options("RavEngine_shaderc_auto", "Automated shader compiler. Do not invoke directly.");
	options.add_options()
	("f,file", "Input JSON shader descriptor", cxxopts::value<string>())
	("o,output", "Output directory for shader", cxxopts::value<string>())
	("i,include", "bgfx/src directory", cxxopts::value<string>());
	
	try{
		auto result = options.parse(argc, argv);
		
		json data;
		path filename = result["file"].as<string>();
		path output = result["output"].as<string>();
		{
			//parse if possible
			ifstream infile(filename);
			if (!infile.is_open()){
				cerr << "ERROR: Could not open descriptor '" << filename << "' for reading."<< endl;
				return 1;
			}
			string str((std::istreambuf_iterator<char>(infile)),std::istreambuf_iterator<char>());
			data = json::parse(str);
		}
		
		path varyingfile = filename.parent_path() / string(data["varying"]);
		path outpath = current_path() / output;
		string includedir = result["include"].as<string>();
		
		string platform, profile;
	#ifdef __APPLE__
		platform = "osx";
		profile = "metal";
	#elif defined _WIN32
		platform = "windows";
		//profile = "spirv";  //do not set a profile on Windows
	#elif defined __linux__
		platform = "linux";
		profle = "spirv";
	#else
		#error Platform not supported
	#endif

		string invocation =
#ifdef _WIN32
			"shaderc.exe";
#elif defined __linux__ || defined __APPLE__
			"./shaderc";
#endif
		
		//make a directory for the shaders
		outpath = outpath / "shaders";
		create_directory(outpath);
		outpath = outpath / filename.filename().replace_extension("");
		create_directory(outpath);
	
		for(json& stage : data["stages"]){
			path input = filename.parent_path() / path(string(stage["file"]));
			string type = stage["stage"];
			path out = outpath / (type+".bin");
			string cmd = invocation + " -f \"" + input.string() + "\" -o \"" + out.string() + "\" -i \"" + includedir + "\" --type " + type + " --platform " + platform + " --varyingdef " + varyingfile.string();

			//no profile on windows directx
			if (platform != "windows") {
				cmd += " --profile " + profile;
			}
			
			//flush before executing
			cout.flush();
			int code = system(cmd.c_str());
			if (code != 0){
				cerr << "Shader compilation failed!" << endl;
				return 2;
			}
		}
	}
	catch(exception& e){
		cerr << "ERROR: " << e.what() << endl;
		return 3;
	}
}
