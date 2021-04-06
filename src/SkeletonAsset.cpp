#include "SkeletonAsset.hpp"
#include <fmt/format.h>
#include <ozz/animation/runtime/skeleton.h>
#include "App.hpp"
#include "Debug.hpp"
#include <ozz/base/io/stream.h>
#include <ozz/base/io/archive.h>
#include <filesystem>

using namespace RavEngine;
using namespace std;

SkeletonAsset::SkeletonAsset(const std::string& str){
	auto path = fmt::format("objects/{}",str);
	
	if(App::Resources->Exists(path.c_str())){
		//is this in ozz format?
		if (filesystem::path(str).extension() == ".ozz"){
			std::vector<uint8_t> data;
			App::Resources->FileContentsAt(path.c_str(),data);
			
			ozz::io::MemoryStream mstr;
			mstr.Write(data.data(), data.size());
			mstr.Seek(0, ozz::io::Stream::kSet);
			
			ozz::io::IArchive archive(&mstr);
			if (archive.TestTag<ozz::animation::Skeleton>()){
				archive >> skeleton;
			}
			else{
				Debug::Fatal("{} is not an animation",path);
			}
		}
		else{
			//TODO: construct skeleton manually by reading file using assimp
		}
	}
	else{
		Debug::Fatal("No skeleton at {}",path);
	}
}
