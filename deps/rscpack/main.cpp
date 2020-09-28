#include <zip_file.hpp>
#include <iostream>
#include <filesystem>
#include <cxxopts.hpp>

using namespace std;
using namespace std::filesystem;

/*
 Usage: rscpack -a AppResourceDir -e EngineResourceDir -o outfile
 */
int main(int argc, char** argv){
	cxxopts::Options options("rscpack", "RavEngine Resource Packer. Do not use directly.");

	options.add_options()
	("a,apprsc", "Directory to built App resources", cxxopts::value<string>())
	("e,engrsc", "Directory to built Engine resources", cxxopts::value<string>())
	("o,out", "Directory to write resource pack", cxxopts::value<string>());
		
	try{
		auto args = options.parse(argc, argv);
		
		path apprsc(args["apprsc"].as<string>());
		path engrsc(args["engrsc"].as<string>());
		path out(args["out"].as<string>());
		
		miniz_cpp::zip_file resources;
		{
			auto shaderpack = [&resources](const path& rootshaderpath){
				if(exists(rootshaderpath)){
					for(const path& item : directory_iterator(rootshaderpath)){
						//go inside the directory
						if (!is_directory(item)){
							resources.write(item, path("shaders") / item.filename());
						}
					}
				}
			};
			//Shader TARs
			shaderpack(apprsc / "shaders");
			shaderpack(engrsc / "shaders");
		}
		auto generalpack = [&resources](const path& rootpath, const path& rootInArchive){
			if(exists(rootpath)){
				for(const path& item : recursive_directory_iterator(rootpath)){
					//make relative path
					path rel = relative(item, rootpath);
					resources.write(item,path("meshes") / rel);
				}
			}
		};
		
		//mesh dir
		generalpack(apprsc / "meshes", "meshes");
		generalpack(engrsc / "meshes", "meshes");
		
		//write final archive
		resources.save(out);
	}
	catch (exception& e){
		cerr << e.what() << endl;
		return -2;
	}
}
