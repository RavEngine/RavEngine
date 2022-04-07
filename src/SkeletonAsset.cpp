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
#include <ozz/animation/runtime/local_to_model_job.h>

using namespace RavEngine;
using namespace std;

SkeletonAsset::SkeletonAsset(const std::string& str){
	auto path = StrFormat("objects/{}",str);
	
	if(GetApp()->GetResources().Exists(path.c_str())){
		//is this in ozz format?
		auto extension = filesystem::path(str).extension();
		if (extension == ".ozz"){
            RavEngine::Vector<uint8_t> data;
			GetApp()->GetResources().FileContentsAt(path.c_str(),data);
			
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
			auto data = GetApp()->GetResources().FileContentsAt(path.c_str());
			const aiScene* scene = aiImportFileFromMemory(reinterpret_cast<char*>(data.data()), Debug::AssertSize<unsigned int>(data.size()),
														  aiProcess_ImproveCacheLocality          |
														  aiProcess_ValidateDataStructure          |
														  aiProcess_FindInvalidData     ,
														  extension.string().c_str());
			
			if (!scene){
				Debug::Fatal("Cannot load: {}", aiGetErrorString());
			}
			
			// create hashset of the bones list to determine quickly if a scene node is a relevant bone
            UnorderedMap<std::string_view,aiBone*> bones;
						
			for(int i = 0; i < scene->mNumMeshes; i++){
				auto mesh = scene->mMeshes[i];	//assume the first mesh is the mesh to use
				if (!mesh->HasBones()){
					continue;
				}
			
				for(int i = 0; i < mesh->mNumBones; i++){
					bones.insert(make_pair(string_view(mesh->mBones[i]->mName.C_Str()),mesh->mBones[i]));
				}
			}
            Debug::Assert(bones.size() > 0, "'{}' does not contain bones!", str);

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
				auto node = scene->mRootNode->FindNode(aiString(name.first.data()));
                
                //if the node has all of the bones in its sub-hierarchy (findnode) then it is the root node for the skeleton
                while (node->mParent != NULL){
                    int found_bones = 0;
                    for(const auto& bone : bones){
                        if (node->FindNode(aiString(bone.first.data()))){
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
                    
                    ozzbone.name = string_view(node->mName.C_Str());
                    
					for(int i = 0; i < node->mNumChildren; i++){
						//is this a relevant bone?
						auto childnode = node->mChildren[i];
						if (bones.contains(string_view(childnode->mName.C_Str()))){
							
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
		
	bindposes.resize(skeleton->joint_names().size());
	stackarray(bindpose_ozz, ozz::math::Float4x4, skeleton->joint_names().size());
	
	//convert from local space to model space
	ozz::animation::LocalToModelJob job;
	job.skeleton = skeleton.get();
	job.input = ozz::span(skeleton->joint_bind_poses());
	job.output = ozz::span(bindpose_ozz, skeleton->joint_names().size_bytes());
	
	Debug::Assert(job.Run(), "Bindpose extraction failed");
	
	//convert to format understandble by GPU
	float matrix[16];
	for(int i = 0; i < skeleton->joint_names().size(); i++){
		auto& t = bindpose_ozz[i];
		for(int r = 0; r < 4; r++){
			float result[4];
			std::memcpy(result,t.cols + r,sizeof(t.cols[r]));
			//_mm_store_ps(result,p.cols[r]);
			std::memcpy(matrix + r*4,result,sizeof(result));
		}
		//inverse here because shader needs the inverse bindpose
		bindposes[i] = glm::inverse(glm::make_mat4(matrix));
	}
	
	assert(bindposes.size() * sizeof(bindposes[0]) < numeric_limits<uint32_t>::max());
	auto bindposedata = bgfx::copy(bindposes.data(), static_cast<uint32_t>(bindposes.size() * sizeof(bindposes[0])));
	
	bindpose = bgfx::createVertexBuffer(bindposedata, layout);
	
	//upload the hierarchy data
	bgfx::VertexLayout hierarchyLayout;
	hierarchyLayout.begin()
		.add(bgfx::Attrib::Position, 1, bgfx::AttribType::Float)
	.end();
	
	// populate hierarchy
	auto parents = skeleton->joint_parents();
	
	stackarray(hierarchy, float, parents.size());
	
	for(int i = 0; i < parents.size(); i++){
		hierarchy[i] = parents[i];
	}
	assert(sizeof(hierarchy) < numeric_limits<uint32_t>::max());	//joint hierarchy is too big!
	boneHierarchy = bgfx::createVertexBuffer(bgfx::copy(hierarchy, static_cast<uint32_t>(sizeof(hierarchy))), hierarchyLayout);
	
}
