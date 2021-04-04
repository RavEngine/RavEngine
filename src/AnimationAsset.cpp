#include "AnimationAsset.hpp"
#include "App.hpp"
#include <ozz/base/io/archive.h>
#include <ozz/base/maths/simd_math.h>
#include <ozz/animation/runtime/blending_job.h>
#include "DataStructures.hpp"
#include <ozz/base/span.h>

using namespace RavEngine;
using namespace std;

AnimationAsset::AnimationAsset(const std::string& name){
	auto path = fmt::format("objects/{}", name);
	if(App::Resources->Exists(path.c_str())){
		std::vector<uint8_t> data;
		App::Resources->FileContentsAt(path.c_str(),data);
		
		ozz::io::MemoryStream mstr;
		mstr.Write(data.data(), data.size());
		mstr.Seek(0, ozz::io::Stream::kSet);
		
		ozz::io::IArchive archive(&mstr);
		if (archive.TestTag<ozz::animation::Animation>()){
			archive >> anim;
		}
		else{
			Debug::Fatal("{} is not an animation",path);
		}
	}
	else{
		Debug::Fatal("No file at {}",path);
	}
}

void AnimationAsset::Sample(float time, ozz::vector<ozz::math::SoaTransform>& locals, ozz::animation::SamplingCache& cache, const ozz::animation::Skeleton& skeleton) const{
	//sample the animation
	ozz::animation::SamplingJob sampling_job;
	sampling_job.animation = &anim;
	sampling_job.cache = &cache;
	sampling_job.ratio = 1;		//TODO: pass correct ratio (delta time)
	sampling_job.output = ozz::make_span(locals);
	
	if (!sampling_job.Run()){
		Debug::Fatal("Sampling job failed");
	}
}

void AnimationClip::Sample(float t, ozz::vector<ozz::math::SoaTransform>& transforms, ozz::animation::SamplingCache& cache, const ozz::animation::Skeleton& skeleton) const{
	//calculate the subtracks
	stackarray(layers, ozz::animation::BlendingJob::Layer, influence.size());
	int index = 0;
	for(auto& row : influence){
		Sampler& sampler = const_cast<Sampler&>(row.second);	//TODO: avoid const_cast
		row.first->Sample(t,sampler.locals,cache,skeleton);
		
		//make sure the buffers are the correct size
		if (sampler.locals.size() != skeleton.num_soa_joints()){
			sampler.locals.resize(skeleton.num_soa_joints());
		}
		
		//populate layers
		layers[index].transform = ozz::make_span(sampler.locals);
		layers[index].weight = sampler.influence;
		index++;
	}
	
	ozz::animation::BlendingJob blend_job;
	blend_job.threshold = 0;			//TODO: make threshold configurable
	blend_job.layers = ozz::span(layers,influence.size());
	blend_job.bind_pose = skeleton.joint_bind_poses();
	
	blend_job.output = make_span(transforms);
	if (!blend_job.Run()){
		Debug::Fatal("Blend job failed");
	}
}
