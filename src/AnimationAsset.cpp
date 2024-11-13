#include "AnimationAsset.hpp"
#include "App.hpp"
#include "Function.hpp"
#include <ozz/base/io/archive.h>
#include <ozz/base/maths/simd_math.h>
#include <ozz/animation/runtime/blending_job.h>
#include <ozz/animation/offline/raw_animation.h>
#include <ozz/animation/offline/animation_builder.h>
#include <ozz/base/maths/soa_transform.h>
#include <ozz/animation/runtime/animation.h>
#include "DataStructures.hpp"
#include <ozz/base/span.h>
#include "Filesystem.hpp"
#include "VirtualFileSystem.hpp"
#include "SkeletonAsset.hpp"
#include "Animation.hpp"

using namespace RavEngine;
using namespace std;

bool AnimationAssetSegment::Sample(float globaltime, float last_global_starttime, float speed, bool looping, ozz::vector<ozz::math::SoaTransform> & transforms, ozz::animation::SamplingJob::Context &cache, const ozz::animation::Skeleton *skeleton) const{
	
	float asset_duration_ticks = (anim_asset->duration_seconds * anim_asset->tps);
		
	float seg_len_sec = (end_ticks - start_ticks)/anim_asset->tps;
	
	float start_unitized = start_ticks / asset_duration_ticks;
	float end_unitized = end_ticks / asset_duration_ticks;
	
	float region = (globaltime - last_global_starttime)/(seg_len_sec/speed);
	
	if (looping){
		region = std::fmod(region,1.f);
	}
	
	float t = region * (end_unitized - start_unitized) + start_unitized;
	bool retval = false;
	if (!looping && t > end_unitized){
		t = end_unitized;
		retval = true;
	}
			
	SampleDirect(t, anim_asset->GetAnim().get(), cache, transforms);
	return retval;
}

template<typename T>
T ReadBytesFromMem(uint8_t*& fp) {
	T val = *reinterpret_cast<T*>(fp);
	fp += sizeof(T);
	return val;
}

template<typename T>
void ReadNFromMem(uint8_t*& fp, T* dest, uint32_t count) {
	auto nbytes = count * sizeof(T);
	std::memcpy(dest, fp, nbytes);
	fp += nbytes;
}

JointAnimation DeserializeJointAnimation(const std::span<uint8_t> data) {
	uint8_t* fp = data.data();
	auto header = ReadBytesFromMem<SerializedJointAnimationHeader>(fp);

	// check header
	if (strncmp(header.header.data(), "rvea", sizeof("rvea") - 1) != 0) {
		Debug::Fatal("Header does not match, data is not a mesh!");
	}

	JointAnimation anim{
		.duration = header.duration,
        .ticksPerSecond = header.ticksPerSecond
	};
	anim.tracks.reserve(header.numTracks);
	anim.name.resize(header.nameLength);

	// read the track name
	memcpy(anim.name.data(), fp, anim.name.size());
	fp += anim.name.size();

	for (int i = 0; i < header.numTracks; i++) {
		auto header = ReadBytesFromMem<SerializedJointAnimationTrackHeader>(fp);
		auto& track = anim.tracks.emplace_back();
		track.translations.resize(header.numTranslations);
		track.rotations.resize(header.numRotations);
		track.scales.resize(header.numScales);

		ReadNFromMem(fp, track.translations.data(), header.numTranslations);
		ReadNFromMem(fp, track.rotations.data(), header.numRotations);
		ReadNFromMem(fp, track.scales.data(), header.numScales);
	}

	return anim;
}

AnimationAsset::AnimationAsset(const std::string& name){
	auto path = Format("animations/{}.rvea", name);
	if(GetApp()->GetResources().Exists(path.c_str())){
		
		auto data = GetApp()->GetResources().FileContentsAt(path.c_str());
		auto anim = DeserializeJointAnimation(data);

		// convert to ozz
		ozz::animation::offline::RawAnimation raw_animation;
		raw_animation.duration = anim.duration;
		raw_animation.name = anim.name;
		raw_animation.tracks.reserve(anim.tracks.size());

        tps = anim.ticksPerSecond;;
		duration_seconds = anim.duration / tps;

		for (const auto& src_track : anim.tracks) {
			auto& track = raw_animation.tracks.emplace_back();
			{
				track.translations.reserve(src_track.translations.size());
				for (const auto& key : src_track.translations) {
					track.translations.push_back({ key.time, {key.value.x, key.value.y, key.value.z} });
				}
			}

			{
				track.rotations.reserve(src_track.rotations.size());
				for (const auto& key : src_track.rotations) {
					track.rotations.push_back({ key.time, {key.value.x, key.value.y, key.value.z, key.value.w} });
				}
			}

			{
				track.scales.reserve(src_track.scales.size());
				for (const auto& key : src_track.scales) {
					track.scales.push_back({ key.time, {key.value.x, key.value.y, key.value.z} });
				}
			}
		}
		Debug::Assert(raw_animation.Validate(),"Animation {} failed validation",name);

		ozz::animation::offline::AnimationBuilder builder;
		this->anim = std::move(builder(raw_animation));
		
	}
	else{
		Debug::Fatal("No file at {}",path);
	}
}

void IAnimGraphable::SampleDirect(float t, const ozz::animation::Animation *anim, ozz::animation::SamplingJob::Context &cache, ozz::vector<ozz::math::SoaTransform> &locals) const{
	//sample the animation
	ozz::animation::SamplingJob sampling_job;
	sampling_job.animation = anim;
	sampling_job.context = &cache;
	sampling_job.ratio = t;
	sampling_job.output = ozz::make_span(locals);
	
	Debug::Assert(sampling_job.Run(), "Sampling job failed");
}

bool AnimationAsset::Sample(float time, float start, float speed, bool looping, ozz::vector<ozz::math::SoaTransform>& locals, ozz::animation::SamplingJob::Context& cache, const ozz::animation::Skeleton* skeleton) const{
	float t = (time - start) / (duration_seconds) * speed;
	bool ret = false;
	if (looping){
		t = std::fmod(t,1.f);
	}
	else {
		ret = t >= 1;
	}
	
	SampleDirect(t, anim.get(), cache, locals);
	return ret;
}

bool AnimationClip::Sample(float t, float start, float speed, bool looping, ozz::vector<ozz::math::SoaTransform>& transforms, ozz::animation::SamplingJob::Context& cache, const ozz::animation::Skeleton* skeleton) const{
	//calculate the subtracks
	stackarray(layers, ozz::animation::BlendingJob::Layer, influence.size());
	int index = 0;
	bool allDone = true;
	for(auto& row : influence){
		Sampler& sampler = const_cast<Sampler&>(row.second);	//TODO: avoid const_cast
		bool done = row.first->Sample(t,start,speed, looping, sampler.locals,cache,skeleton);
		if (!done) {
			allDone = false;
		}
		
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
	blend_job.rest_pose = skeleton->joint_rest_poses();
	
	blend_job.output = make_span(transforms);
	if (!blend_job.Run()){
		Debug::Fatal("Blend job failed");
	}
	return allDone;
}

bool RavEngine::CustomSkeletonAnimation::Sample(float t, float start, float speed, bool looping, ozz::vector<ozz::math::SoaTransform>& locals, ozz::animation::SamplingJob::Context& cache, const ozz::animation::Skeleton* skeleton) const
{
	return mutateBonesHook(ozz::make_span(locals), skeleton, t, start, speed, looping);
}
