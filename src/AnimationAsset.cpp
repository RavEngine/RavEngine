#include "AnimationAsset.hpp"
#include "App.hpp"
#include <ozz/base/io/archive.h>
#include <ozz/base/maths/simd_math.h>
#include <ozz/animation/runtime/blending_job.h>
#include <ozz/animation/offline/raw_animation.h>
#include <ozz/animation/offline/animation_builder.h>
#include "DataStructures.hpp"
#include <ozz/base/span.h>
#include <filesystem>
#include <assimp/cimport.h>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

using namespace RavEngine;
using namespace std;

AnimationAsset::AnimationAsset(const std::string& name, Ref<SkeletonAsset> skeleton){
	auto path = fmt::format("objects/{}", name);
	if(App::Resources->Exists(path.c_str())){
		//is this in ozz format
		auto extension = filesystem::path(name).extension();
		if (extension == ".ozz"){
			std::vector<uint8_t> data;
			App::Resources->FileContentsAt(path.c_str(),data);
			
			ozz::io::MemoryStream mstr;
			mstr.Write(data.data(), data.size());
			mstr.Seek(0, ozz::io::Stream::kSet);
			
			anim = ozz::make_unique<ozz::animation::Animation>();
			
			ozz::io::IArchive archive(&mstr);
			if (archive.TestTag<ozz::animation::Animation>()){
				archive >> *anim;
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
						
			Debug::Assert(scene->HasAnimations(), "Scene must have animations");
			ozz::animation::offline::RawAnimation raw_animation;
			
			auto& animations = scene->mAnimations;
			
			//find the longest animation, this is the length of the raw anim
			
			//assume the first animation is the animation to use
			auto anim = scene->mAnimations[0];
			raw_animation.duration = anim->mDuration * anim->mTicksPerSecond;
			
			auto create_keyframe = [&](const aiNodeAnim* channel, ozz::animation::offline::RawAnimation::JointTrack& track){
				
				//translate
				for(int i = 0; i < channel->mNumPositionKeys; i++){
					auto key = channel->mPositionKeys[i];
					track.translations.push_back({ static_cast<float>(key.mTime), ozz::math::Float3(key.mValue.x,key.mValue.y,key.mValue.z) });
				}
				
				//rotate
				for(int i = 0; i < channel->mNumRotationKeys; i++){
					auto key = channel->mRotationKeys[i];
					track.rotations.push_back({ static_cast<float>(key.mTime), ozz::math::Quaternion(key.mValue.x,key.mValue.y,key.mValue.z, key.mValue.w) });
				}
				
				//scale
				for(int i = 0; i < channel->mNumScalingKeys; i++){
					auto key = channel->mScalingKeys[i];
					track.scales.push_back({ static_cast<float>(key.mTime), ozz::math::Float3(key.mValue.x,key.mValue.y,key.mValue.z) });
				}
			};
			
			// populate the tracks
			raw_animation.tracks.resize(skeleton->GetSkeleton()->num_joints());
			raw_animation.name = string(anim->mName.C_Str());
			for(int i = 0; i < anim->mNumChannels; i++){
				auto channel = anim->mChannels[i];
				
				//tracks must be ordered the same way they are stored in the skeleton
				//search for the bone in the skeleton, and use its index
				
				auto names = skeleton->GetSkeleton()->joint_names();
				
				std::string bonename(channel->mNodeName.C_Str());
				
				auto it = std::find(names.begin(), names.end(), bonename);
				
				//this track is not relevant to this skeleton, so ignore it
				if (it == names.end()){
					continue;
				}
				
				//calculate index
				auto bone_index = it - names.begin();
				
				//populate the keyframes per track
				create_keyframe(channel, raw_animation.tracks[bone_index]);
				
			}
			
			//free afterward
			aiReleaseImport(scene);
			
			Debug::Assert(raw_animation.Validate(), "Animation validation failed");
			
			ozz::animation::offline::AnimationBuilder builder;
			this->anim = std::move(builder(raw_animation));
		}
	}
	else{
		Debug::Fatal("No file at {}",path);
	}
}

void AnimationAsset::Sample(float time, ozz::vector<ozz::math::SoaTransform>& locals, ozz::animation::SamplingCache& cache, const ozz::animation::Skeleton* skeleton) const{
	//sample the animation
	ozz::animation::SamplingJob sampling_job;
	sampling_job.animation = anim.get();
	sampling_job.cache = &cache;
	sampling_job.ratio = time;
	sampling_job.output = ozz::make_span(locals);
	
	Debug::Assert(sampling_job.Run(), "Sampling job failed");
}

void AnimationClip::Sample(float t, ozz::vector<ozz::math::SoaTransform>& transforms, ozz::animation::SamplingCache& cache, const ozz::animation::Skeleton* skeleton) const{
	//calculate the subtracks
	stackarray(layers, ozz::animation::BlendingJob::Layer, influence.size());
	int index = 0;
	for(auto& row : influence){
		Sampler& sampler = const_cast<Sampler&>(row.second);	//TODO: avoid const_cast
		row.first->Sample(t,sampler.locals,cache,skeleton);
		
		//make sure the buffers are the correct size
		if (sampler.locals.size() != skeleton->num_soa_joints()){
			sampler.locals.resize(skeleton->num_soa_joints());
		}
		
		//populate layers
		layers[index].transform = ozz::make_span(sampler.locals);
		layers[index].weight = sampler.influence;
		index++;
	}
	
	ozz::animation::BlendingJob blend_job;
	blend_job.threshold = 0;			//TODO: make threshold configurable
	blend_job.layers = ozz::span(layers,influence.size());
	blend_job.bind_pose = skeleton->joint_bind_poses();
	
	blend_job.output = make_span(transforms);
	if (!blend_job.Run()){
		Debug::Fatal("Blend job failed");
	}
}
