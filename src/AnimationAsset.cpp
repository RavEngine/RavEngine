#include "AnimationAsset.hpp"
#include "App.hpp"
#include <ozz/base/io/archive.h>

using namespace RavEngine;
using namespace std;

AnimationAsset::AnimationAsset(const std::string& name){
	if(App::Resources->Exists(name.c_str())){
		std::vector<uint8_t> data;
		App::Resources->FileContentsAt(name.c_str(),data);
		
		ozz::io::MemoryStream mstr;
		mstr.Read(data.data(), data.size());
		
		ozz::io::IArchive archive(&mstr);
		if (archive.TestTag<ozz::animation::Animation>()){
			archive >> anim;
		}
		else{
			Debug::Fatal("{} is not an animation",name);
		}
	}
}
