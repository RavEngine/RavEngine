#pragma once
#include "Ref.hpp"
#include "Map.hpp"
#include "SpinLock.hpp"
#include <ozz/base/containers/vector.h>
#include <ozz/animation/runtime/sampling_job.h>
#include <ozz/base/memory/unique_ptr.h>
#include "Function.hpp"
#include "Manager.hpp"
#include "mathtypes.hpp"

namespace ozz::animation {
	struct Skeleton;
	struct Animation;
}

namespace ozz::math {
	struct SoaTransform;
}

namespace RavEngine{

	class SkeletonAsset;

/**
* A node in an animation graph.
*/
struct IAnimGraphable{
	/**
	 Sample the animation curves
	 @param t the time to sample (same units as @code GetCurrentTime() @endcode)
	 @param output the vector to write the output transforms to
	 @param cache a sampling cache, modified when used
	 @return true if the clip has ended, false otherwise
	 */
	virtual bool Sample(float t, float start,
						float speed,
						bool looping,
						ozz::vector<ozz::math::SoaTransform>& output,
						ozz::animation::SamplingJob::Context& cache,
						const ozz::animation::Skeleton* skeleton) const = 0;
	
	
	/**
	* Executes the SamplingJob directly. Used for when a sampling result is needed in @code Sample @endcode.
	* @param t the time to sample (same units as @code GetCurrentTime() @endcode)
	* @param anim the ozz animation to sample
	* @param cache the cache context to use for sampling
	* @param locals the vector to write the model-space bone transforms
	*/
    void SampleDirect(float t, const ozz::animation::Animation* anim, ozz::animation::SamplingJob::Context& cache, ozz::vector<ozz::math::SoaTransform>& locals) const;
};

/**
* Represents a pre-computed animation track
*/
class AnimationAsset : public IAnimGraphable{
	//duration
	//clip data
	ozz::unique_ptr<ozz::animation::Animation> anim;
public:
	AnimationAsset(const std::string& name);
	
	/**
	 Sample the animation curves
	 @param t the time to sample
	 @param output the vector to write the output transforms to
	 @param cache a sampling cache, modified when used
	 */
	bool Sample(float t, float start, float speed, bool looping, ozz::vector<ozz::math::SoaTransform>&, ozz::animation::SamplingJob::Context& cache, const ozz::animation::Skeleton* skeleton) const override;
	
	/**
	* @return the Ozz animation object
	*/
	constexpr inline const decltype(anim)& GetAnim() const{
		return anim;
	}
	
	float duration_seconds;
	float tps;
};

class BoneTransforms{
    ozz::span<ozz::math::SoaTransform> transforms;
public:
    BoneTransforms(decltype(transforms) t) : transforms(t){}
    
    
    struct SingleTransform{
        quaternion rotation;
        vector3 translation, scale;
    };
    
    SingleTransform GetBone(uint32_t index) const;
    void SetBone(uint32_t index, const SingleTransform&);
};

/**
* An animation tree node that enables games to provide code-driven animation
*/
class CustomSkeletonAnimation : public IAnimGraphable {
	Function<bool(BoneTransforms, const ozz::animation::Skeleton*, float, float, float, bool)> mutateBonesHook;
	bool Sample(float t, float start, float speed, bool looping, ozz::vector<ozz::math::SoaTransform>&, ozz::animation::SamplingJob::Context& cache, const ozz::animation::Skeleton* skeleton) const override;
public:
	/**
	* Provide a custom Callable that provides bone transformations. See CustomSkeletonAnimationFunction for parameters.
	* @param hook the function to call to provide bone data
	*/
	CustomSkeletonAnimation(const decltype(mutateBonesHook)& hook) : mutateBonesHook(hook) {}
};

struct CustomSkeletonAnimationFunction {

	/** Derive from this class to provide custom animations.
	* @param transforms the bone transforms. Write changes here
	* @param skeleton the skeleton representing the transforms.
	* @param t the current time
	* @param start the time the animation was last "Play"ed 
	* @param speed the playback rate of the animation where 1.0 is normal speed
	* @param looping whether the animation should loop
	* @return true if the animation has completed, false otherwise.
	*/
	virtual bool operator()(BoneTransforms transforms, const ozz::animation::Skeleton* skeleton, float t, float start, float end, bool loop) = 0;

	virtual ~CustomSkeletonAnimationFunction() {}
};

template<typename T>
struct LambdaSkeletonAnimationFunction : public CustomSkeletonAnimationFunction {
	T fn;

	LambdaSkeletonAnimationFunction(const decltype(fn)& f) : fn(f) {}

	bool operator()(BoneTransforms transforms, const ozz::animation::Skeleton* skeleton, float t, float start, float end, bool loop) final {
		return fn(transforms, skeleton, t, start, end, loop);
	}
};

/**
* Represents a subclip within a larger animation asset. The closest analogue to other engines is the customizable animation clip ranges in the Unity animation importer.
*/
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
	
	bool Sample(float global_time, float last_globalplaytime, float speed, bool looping, ozz::vector<ozz::math::SoaTransform>&, ozz::animation::SamplingJob::Context& cache, const ozz::animation::Skeleton* skeleton) const override;
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
	bool Sample(float t, float start, float speed, bool looping, ozz::vector<ozz::math::SoaTransform>&, ozz::animation::SamplingJob::Context& cache, const ozz::animation::Skeleton* skeleton) const override;
	
};

struct AnimationAssetManager : public GenericWeakReadThroughCache<std::string, AnimationAsset> {};

}
