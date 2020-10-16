#include <iostream>
#include <fstream>
#include <cxxopts.hpp>
#include <nlohmann/json.hpp>
#include <filesystem>
#include <ravtar/tarball.hpp>

#ifdef _WIN32
#include <Windows.h>
#include <process.h>
#else
#include <unistd.h>
#endif

using namespace std;
using namespace nlohmann;
using namespace Tar;
using namespace std::filesystem;

string dx_profileprefix(const std::string& stage) {
	if (stage == "vertex") {
		return "v";		//vertex
	}
	else if (stage == "fragment") {
		return "p";		//fragment
	}
	else {
		return "c";		//compute
	}
}

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
		
		string varyingfile = (filename.parent_path() / string(data["varying"])).string();
		path outpath = current_path() / output;
		string includedir = result["include"].as<string>();
		
		string platform, profile;
	#ifdef __APPLE__
		platform = "osx";
		profile = "metal";
	#elif defined _WIN32
		platform = "windows";
		profile = "s_5";  //do not set a profile on Windows
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
		outpath = outpath / "shaders" / filename.filename().replace_extension("");
		create_directories(outpath);
		
		const auto tarpath = outpath.parent_path() / filename.filename().replace_extension("tar");
		
		ofstream outtar(tarpath);
		TarWriter tarball(outtar);
	
		for(json& stage : data["stages"]){
			string input = (filename.parent_path() / path(string(stage["file"]))).string();
			string type = stage["stage"];
			path out = outpath / (type+".bin");
			
			string o = out.string();
			
			string pstr = (platform != "windows") ?  profile : (dx_profileprefix(type) + profile + "_0");
			
			char* args[] {
				(char*)"-f",
				const_cast<char*>(input.c_str()),
				(char*)"-o",
				const_cast<char*>(o.c_str()),
				(char*)"-i",
				const_cast<char*>(includedir.c_str()),
				(char*)"--type",
				const_cast<char*>(type.c_str()),
				(char*)"--platform",
				const_cast<char*>(platform.c_str()),
				(char*)"--varyingdef",
				const_cast<char*>(varyingfile.c_str()),
				(char*)"--profile",
				const_cast<char*>(pstr.c_str())
			};
			
#ifdef _WIN32
			PROCESS_INFORMATION ProcessInfo;
			STARTUPINFO startupInfo;

			ZeroMemory(&startupInfo, sizeof(startupInfo));
			startupInfo.cb = sizeof startupInfo; //Only compulsory field

			//create command line string
			std::unordered_set<unsigned int> pathIds = {1,3,5,11};
			std::string cmd = invocation;
			for (int i = 0; i < sizeof(args) / sizeof(args[0]); i++) {
				if (pathIds.find(i) != pathIds.end()) {
					cmd += " \"" + string(args[i]) + "\"";
				}
				else {
					cmd += " " + string(args[i]);
				}
			}
			DWORD exit_code;
			if (CreateProcess(invocation.c_str(), LPSTR(cmd.c_str()), NULL, NULL, FALSE,0, NULL, NULL, &startupInfo, &ProcessInfo)) {
				WaitForSingleObject(ProcessInfo.hProcess, INFINITE);
				GetExitCodeProcess(ProcessInfo.hProcess, &exit_code);
				if (exit_code != 0) {
					cerr << "Shader compilation failed!" << endl;
					exit(2);
				}

				CloseHandle(ProcessInfo.hThread);
				CloseHandle(ProcessInfo.hProcess);
			}
			else {
				std::cerr << "Failed to launch shader compiler" << endl;
				exit(3);
			}
#else
			//spawn worker child process
			pid_t pid = fork();
			int status;
			if (pid == 0) {
				//in child process
				execv(invocation.c_str(), args);
				//an error occured if code still running
				cerr << "Shader compilation failed!" << endl;
				//delete the tar
				filesystem::remove(tarpath);
				exit(2);
			}
			else {
				//in parent process
				//wait for child to terminate
				pid_t pidc;
				do {
					pidc = wait(&status);
					if (pidc != pid) {
						break;	//if child stopped, parent can continue
					}
				} while (pidc != pid);
			}
#endif
						
			//add to TAR
			tarball.putFile(out.string().c_str(),out.filename().string().c_str());
		}
		tarball.finish();
		outtar.close();
	}
	catch(exception& e){
		cerr << "ERROR: " << e.what() << endl;
		return 3;
	}
}
