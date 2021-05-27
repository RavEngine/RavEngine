#pragma once
#include <phmap.h>
#include "Ref.hpp"
#include "DataStructures.hpp"
#include "SpinLock.hpp"
#include <ozz/animation/runtime/animation.h>
#include <ozz/base/containers/vector.h>
#include <ozz/base/maths/soa_transform.h>
#include <ozz/animation/runtime/sampling_job.h>
#include "SkeletonAsset.hpp"

namespace RavEngine{

struct IAnimGraphable{
	/**
	 Sample the animation curves
	 @param t the time to sample
	 @param output the vector to write the output transforms to
	 @param cache a sampling cache, modified when used
	 */
	virtual void Sample(float t, float start,
						float speed,
						bool looping,
						ozz::vector<ozz::math::SoaTransform>& output,
						ozz::animation::SamplingCache& cache,
						const ozz::animation::Skeleton* skeleton) const = 0;	//TODO: make abstract
};


class AnimationAsset : public IAnimGraphable{
	//duration
	//clip data
	ozz::unique_ptr<ozz::animation::Animation> anim;
	//method to calculate curves given a time point
	float duration;
	float tps;
public:
	AnimationAsset(const std::string& name, Ref<SkeletonAsset> skeleton);
	
	/**
	 Sample the animation curves
	 @param t the time to sample
	 @param output the vector to write the output transforms to
	 @param cache a sampling cache, modified when used
	 */
	void Sample(float t, float start, float speed, bool looping, ozz::vector<ozz::math::SoaTransform>&, ozz::animation::SamplingCache& cache, const ozz::animation::Skeleton* skeleton) const override;
};

class AnimationAssetSegment : public IAnimGraphable{
public:
	float start_time, end_time;
	Ref<AnimationAsset> anim_asset;
	AnimationAssetSegment(decltype(anim_asset) asset, float start, float end) : anim_asset(asset), start_time(start), end_time(end){}
	
	void Sample(float t, float start, float speed, bool looping, ozz::vector<ozz::math::SoaTransform>&, ozz::animation::SamplingCache& cache, const ozz::animation::Skeleton* skeleton) const override;
};

class AnimationClip : public IAnimGraphable{
	struct Sampler{
		float influence;
		ozz::vector<ozz::math::SoaTransform> locals;
	};
	locked_hashmap<Ref<IAnimGraphable>,Sampler, SpinLock> influence;
public:
	/**
	 Add an AnimationAsset to the collection, or change the influence for the existing asset
	 @param inf the influence for this clip
	 */
	inline void SetAnimationInfluence(Ref<IAnimGraphable> asset, float inf = 1){
		influence[asset].influence = inf;
	}
	
	/**
	 Remove an animation from the collection
	 @param asset the asset to remove
	 */
	inline void RemoveAnimation(Ref<IAnimGraphable> asset){
		influence.erase(asset);
	}
	
	inline void Clear(){
		influence.clear();
	}
	
	inline bool IsEmpty() const{
		return influence.empty();
	}
	
	/**
	 Sample the animation curves across all clips
	 @param t the time to sample
	 @param output the vector to write the output transforms to
	 @param cache a sampling cache, modified when used
	 */
	void Sample(float t, float start, float speed, bool looping, ozz::vector<ozz::math::SoaTransform>&, ozz::animation::SamplingCache& cache, const ozz::animation::Skeleton* skeleton) const override;
	
};
}
