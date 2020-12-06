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
#include <sys/types.h>
#include <sys/wait.h>
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
	("n,name", "Name of shader", cxxopts::value<string>())
	("v,vertex", "Path to vertex shader source", cxxopts::value<string>())
	("f,fragment", "Path to fragment shader source", cxxopts::value<string>())
	("d,varying", "Path to varying.def source", cxxopts::value<string>())
	("o,output", "Output directory for shader", cxxopts::value<string>())
	("i,include", "bgfx/src directory", cxxopts::value<string>());
	
	try{
		auto result = options.parse(argc, argv);
		
		path output = result["output"].as<string>();
		
		string varyingfile = (result["varying"].as<string>());
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
		profile = "spirv";
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
		outpath = outpath / "shaders" / result["name"].as<string>();
		create_directories(outpath);
		
		const auto tarpath = outpath.parent_path() / (result["name"].as<string>() + ".tar");
		//remove old TAR, so that if the compile fails, the engine will not load
		filesystem::remove(tarpath);
				
		struct stage{
			string file;
			string type;
		};
		
		stage stages[] = {{result["vertex"].as<string>(),"vertex"},{result["fragment"].as<string>(),"fragment"}};

		struct file {
			string name;
			string path;
			file(const string& p, const string& n) : name(n), path(p) {};
		};
		vector<file> paths;
	
		for(stage& stage : stages){
			string input = stage.file;
			string type = stage.type;
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
				const_cast<char*>(pstr.c_str()),
#ifndef _WIN32
				NULL
#endif
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
			cout << cmd << endl;
			DWORD exit_code;
			if (CreateProcess(invocation.c_str(), LPSTR(cmd.c_str()), NULL, NULL, FALSE,0, NULL, NULL, &startupInfo, &ProcessInfo)) {
				WaitForSingleObject(ProcessInfo.hProcess, INFINITE);
				GetExitCodeProcess(ProcessInfo.hProcess, &exit_code);
				if (exit_code != 0) {
					cerr << "Shader compilation failed!" << endl;
					
					CloseHandle(ProcessInfo.hThread);
					CloseHandle(ProcessInfo.hProcess);
					exit(2);
				}

				CloseHandle(ProcessInfo.hThread);
				CloseHandle(ProcessInfo.hProcess);
			}
			else {
				cerr << "Failed to launch shader compiler" << endl;
				exit(3);
			}
#else
			//spawn worker child process
			pid_t pid = fork();
			int status;
			int link[2];
			if (pid == 0) {
				//in child process
				execv(invocation.c_str(), args);
				//an error occured if code still running
				cerr << "Shader compilation failed!" << endl;
				//print output
				fprintf(stderr, "%s\n", "execv");
				
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

			//add to path
			paths.emplace_back(out.string(),out.filename().string());
		}
		for(const auto& p : paths){
			if (!exists(p.path)){
				cerr << "ERROR: shader file at '" << p.path << "' was not created" << endl;
				exit(2);
			}
		}
		
		ofstream outtar(tarpath);
		TarWriter tarball(outtar);
		//create the TAR
		for (const auto& p : paths) {
			tarball.putFile(p.path.c_str(),p.name.c_str());
		}

		tarball.finish();
		outtar.close();
	}
	catch(exception& e){
		cerr << "ERROR: " << e.what() << endl;
		return 3;
	}
}
