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
	 @return true if the clip has ended, false otherwise
	 */
	virtual bool Sample(float t, float start,
						float speed,
						bool looping,
						ozz::vector<ozz::math::SoaTransform>& output,
						ozz::animation::SamplingCache& cache,
						const ozz::animation::Skeleton* skeleton) const = 0;	//TODO: make abstract
	
	
    void SampleDirect(float t, const ozz::animation::Animation* anim, ozz::animation::SamplingCache& cache, ozz::vector<ozz::math::SoaTransform>& locals) const;
};


class AnimationAsset : public IAnimGraphable{
	//duration
	//clip data
	ozz::unique_ptr<ozz::animation::Animation> anim;
public:
	AnimationAsset(const std::string& name, Ref<SkeletonAsset> skeleton);
	
	/**
	 Sample the animation curves
	 @param t the time to sample
	 @param output the vector to write the output transforms to
	 @param cache a sampling cache, modified when used
	 */
	bool Sample(float t, float start, float speed, bool looping, ozz::vector<ozz::math::SoaTransform>&, ozz::animation::SamplingCache& cache, const ozz::animation::Skeleton* skeleton) const override;
	
	constexpr inline const decltype(anim)& GetAnim() const{
		return anim;
	}
	
	float duration_seconds;
	float tps;
};

class AnimationAssetSegment : public IAnimGraphable{
public:
	float start_ticks, end_ticks;
	Ref<AnimationAsset> anim_asset;
	
	/**
	 Create an animation segment from an existing AnimationAsset
	 @param start the start time of the animation, in frames (see your DCC app)
	 @param end the number of frames to remove from the end of the animation (like an end trim)
	 */
	AnimationAssetSegment(decltype(anim_asset) asset, float start, float end = 0) : anim_asset(asset), start_ticks(start), end_ticks(end){}
	
	bool Sample(float global_time, float last_globalplaytime, float speed, bool looping, ozz::vector<ozz::math::SoaTransform>&, ozz::animation::SamplingCache& cache, const ozz::animation::Skeleton* skeleton) const override;
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
	
    constexpr inline void Clear(){
		influence.clear();
	}
	
    constexpr inline bool IsEmpty() const{
		return influence.empty();
	}
	
	/**
	 Sample the animation curves across all clips
	 @param t the time to sample
	 @param output the vector to write the output transforms to
	 @param cache a sampling cache, modified when used
	 */
	bool Sample(float t, float start, float speed, bool looping, ozz::vector<ozz::math::SoaTransform>&, ozz::animation::SamplingCache& cache, const ozz::animation::Skeleton* skeleton) const override;
	
};
}
