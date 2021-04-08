#include "SkeletonAsset.hpp"
#include <fmt/format.h>
#include <ozz/animation/runtime/skeleton.h>
#include <ozz/animation/offline/raw_skeleton.h>
#include "App.hpp"
#include "Debug.hpp"
#include <ozz/base/io/stream.h>
#include <ozz/base/io/archive.h>
#include <filesystem>
#include <assimp/cimport.h>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <ozz/base/memory/unique_ptr.h>

using namespace RavEngine;
using namespace std;

SkeletonAsset::SkeletonAsset(const std::string& str){
	auto path = fmt::format("objects/{}",str);
	
	if(App::Resources->Exists(path.c_str())){
		//is this in ozz format?
		auto extension = filesystem::path(str).extension();
		if (extension == ".ozz"){
			std::vector<uint8_t> data;
			App::Resources->FileContentsAt(path.c_str(),data);
			
			ozz::io::MemoryStream mstr;
			mstr.Write(data.data(), data.size());
			mstr.Seek(0, ozz::io::Stream::kSet);
			
			skeleton = ozz::make_unique<ozz::animation::Skeleton>();
			
			ozz::io::IArchive archive(&mstr);
			if (archive.TestTag<ozz::animation::Skeleton>()){
				archive >> *skeleton;
			}
			else{
				Debug::Fatal("{} is not an animation",path);
			}
		}
		else{
			auto data = App::Resources->FileContentsAt(path.c_str());
			const aiScene* scene = aiImportFileFromMemory(data.data(), data.size(),
														  aiProcess_ImproveCacheLocality          |
														  aiProcess_ValidateDataStructure          |
														  aiProcess_FindInvalidData     ,
														  extension.string().c_str());
			
			if (!scene){
				Debug::Fatal("Cannot load: {}", aiGetErrorString());
			}
			
			// create hashset of the bones list to determine quickly if a scene node is a relevant bone
			phmap::flat_hash_map<std::string,aiBone*> bones;
			
			plf::list<aiString> allBoneNames;
			
			for(int i = 0; i < scene->mNumMeshes; i++){
				auto mesh = scene->mMeshes[i];	//assume the first mesh is the mesh to use
				if (!mesh->HasBones()){
					continue;
				}
			
				for(int i = 0; i < mesh->mNumBones; i++){
					bones.insert(make_pair(string(mesh->mBones[i]->mName.C_Str()),mesh->mBones[i]));
					allBoneNames.push_back(mesh->mBones[i]->mName);
				}
			}
			
			// we will now bone this mesh
			
			//recurse the root node and get all of the bones
			ozz::animation::offline::RawSkeleton raw_skeleton;
			raw_skeleton.roots.resize(1);
			auto& root = raw_skeleton.roots[0];
			
			//find root bone
			aiNode* rootBone = nullptr;;
			for(const auto& name : allBoneNames){
				auto node = scene->mRootNode->FindNode(name);
				//no parent, or parent is not in the bones list?
				if (node->mParent == NULL || !bones.contains(string(node->mParent->mName.C_Str())) ){
					rootBone = node;
					break;
				}
			}
			
			Debug::Assert(rootBone != nullptr, "Could not find root bone");
			
			//construct skeleton hierarchy
			auto recurse_bone = [&bones](decltype(root)& ozzbone, aiNode* node) -> void{
				auto recurse_impl = [&bones](decltype(root)& ozzbone, aiNode* node, auto& recursive_call) -> void{
					for(int i = 0; i < node->mNumChildren; i++){
						//is this a relevant bone?
						if (bones.contains(string(node->mChildren[i]->mName.C_Str()))){
							
							//create a new bone
							auto& newbone = ozzbone.children.emplace_back();
							
							// create its transformation
							aiVector3t<float> scale, position;
							aiQuaterniont<float> rotation;
							node->mTransformation.Decompose(scale, rotation, position);
							newbone.transform.translation = ozz::math::Float3(position.x,position.y,position.z);
							newbone.transform.scale = ozz::math::Float3(scale.x,scale.y,scale.z);
							newbone.transform.rotation = ozz::math::Quaternion(rotation.x, rotation.y, rotation.z, rotation.w);
							
							newbone.name = string(node->mChildren[i]->mName.C_Str());
							
							//construct all the child bones for this bone
							recursive_call(newbone,node->mChildren[i],recursive_call);
						}
						//construct all the child bones for this bone
						recursive_call(ozzbone,node->mChildren[i],recursive_call);
					}
				};
				recurse_impl(ozzbone, node, recurse_impl);
			};
			
			recurse_bone(root, rootBone);
			
			//free afterward
			aiReleaseImport(scene);
			
			//convert into a runtime-optimized skeleton
			Debug::Assert(raw_skeleton.Validate(), "Skeleton validation failed");
			
			ozz::animation::offline::SkeletonBuilder skbuilder;
			ozz::unique_ptr<ozz::animation::Skeleton> sk = skbuilder(raw_skeleton);
			skeleton = std::move(sk);
		}
	}
	else{
		Debug::Fatal("No skeleton at {}",path);
	}
}
