#include <zip_file.hpp>
#include <iostream>
#include <filesystem>
#include <cxxopts.hpp>

using namespace std;
using namespace std::filesystem;

std::string unix_path(const std::filesystem::path& p) {
#ifdef _WIN32
	auto prelim = p.string();
	std::replace(prelim.begin(), prelim.end(), '\\', '/');
	return prelim;
#else
	return p.string();
#endif
}

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
							auto inarch = unix_path((path("shaders") / item.filename()).string());
							resources.write(item.string(), inarch);
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
					resources.write(item.string(),unix_path((rootInArchive / rel).string()));
				}
			}
		};
		
		//mesh dir
		generalpack(apprsc / "meshes", "meshes");
		generalpack(engrsc / "meshes", "meshes");
		generalpack(apprsc / "textures", "textures");
		
		//write final archive
		create_directories(out.parent_path());
		resources.save(out.string());
	}
	catch (exception& e){
		cerr << e.what() << endl;
		return -2;
	}
}
