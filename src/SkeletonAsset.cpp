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
#include <ozz/base/span.h>
#include <ozz/base/maths/soa_transform.h>

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
						
			for(int i = 0; i < scene->mNumMeshes; i++){
				auto mesh = scene->mMeshes[i];	//assume the first mesh is the mesh to use
				if (!mesh->HasBones()){
					continue;
				}
			
				for(int i = 0; i < mesh->mNumBones; i++){
					bones.insert(make_pair(string(mesh->mBones[i]->mName.C_Str()),mesh->mBones[i]));
				}
			}
			
			// we will now bone this mesh
			
			//recurse the root node and get all of the bones
			ozz::animation::offline::RawSkeleton raw_skeleton;
			raw_skeleton.roots.resize(1);
			auto& root = raw_skeleton.roots[0];
			
			//find root bone
            //pick a bone, and recurse up its parent hierarchy
            //TODO: this is very inefficient, optimize
			aiNode* rootBone = nullptr;;
			for(const auto& name : bones){
				auto node = scene->mRootNode->FindNode(aiString(name.first.c_str()));
                
                //if the node has all of the bones in its sub-hierarchy (findnode) then it is the root node for the skeleton
                while (node->mParent != NULL){
                    int found_bones = 0;
                    for(const auto& bone : bones){
                        if (node->FindNode(aiString(bone.first.c_str()))){
                            found_bones++;
                        }
                    }
                    if (found_bones == bones.size()){
                        rootBone = node;
                        goto found_bone;
                    }
                    else{
                        node = node->mParent;
                    }
                }
                
			}
			found_bone:
			Debug::Assert(rootBone != nullptr, "Could not find root bone");
			
			//construct skeleton hierarchy
			auto recurse_bone = [&bones](decltype(root)& ozzbone, aiNode* node) -> void{
				auto recurse_impl = [&bones](decltype(root)& ozzbone, aiNode* node, auto& recursive_call) -> void{
                    // create its transformation
                    aiVector3t<float> scale, position;
                    aiQuaterniont<float> rotation;
                    node->mTransformation.Decompose(scale, rotation, position);
                    ozzbone.transform.translation = ozz::math::Float3(position.x,position.y,position.z);
                    ozzbone.transform.scale = ozz::math::Float3(scale.x,scale.y,scale.z);
                    ozzbone.transform.rotation = ozz::math::Quaternion(rotation.x, rotation.y, rotation.z, rotation.w);
                    
                    ozzbone.name = string(node->mName.C_Str());
                    
					for(int i = 0; i < node->mNumChildren; i++){
						//is this a relevant bone?
						auto childnode = node->mChildren[i];
						if (bones.contains(string(childnode->mName.C_Str()))){
							
							//create a new bone
							auto& newbone = ozzbone.children.emplace_back();
							
							//construct all the child bones for this bone
							recursive_call(newbone,childnode,recursive_call);
						}
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
	
	bgfx::VertexLayout layout;
	layout.begin()
		.add(bgfx::Attrib::Position, 4, bgfx::AttribType::Float)
		.add(bgfx::Attrib::Position, 4, bgfx::AttribType::Float)
		.add(bgfx::Attrib::Position, 4, bgfx::AttribType::Float)
		.add(bgfx::Attrib::Position, 4, bgfx::AttribType::Float)
	.end();
		
	stackarray(bindposes, glm::mat4, skeleton->joint_bind_poses().size());
	std::memset(bindposes, 0, sizeof(bindposes));
	
#define vf2f(a,b) std::memcpy(&a, &b, sizeof(a))
	
	for(int i = 0; i < skeleton->joint_bind_poses().size(); i++){
		auto& bp = skeleton->joint_bind_poses()[i];
		
		float sx, sy, sz,
			  rx, ry, rz, rw,
			  tx, ty, tz;
		
		vf2f(sx, bp.scale.x);
		vf2f(sx, bp.scale.y);
		vf2f(sz, bp.scale.z);
		
		vf2f(rx, bp.rotation.x);
		vf2f(ry, bp.rotation.y);
		vf2f(rz, bp.rotation.z);
		vf2f(rw, bp.rotation.w);
		
		vf2f(tx, bp.translation.x);
		vf2f(ty, bp.translation.y);
		vf2f(tz, bp.translation.z);
		
		glm::translate(glm::mat4(1), glm::vec3(tx,ty,tz)) * glm::toMat4(glm::quat(rx,ry,rz,rw)) * glm::scale(glm::mat4(1), glm::vec3(sx,sy,sz));
	}
	
	auto bindposedata = bgfx::copy(bindposes, sizeof(bindposes));
	
	bindpose = bgfx::createVertexBuffer(bindposedata, layout);
}
