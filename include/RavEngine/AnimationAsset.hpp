#pragma once
#include <phmap.h>
#include "Ref.hpp"
#include "DataStructures.hpp"
#include "SpinLock.hpp"
#include <ozz/animation/runtime/animation.h>

namespace RavEngine{

struct IAnimGraphable{
	virtual void Calculate() const {}	//TODO: make abstract
};


class AnimationAsset : public IAnimGraphable{
	//duration
	//clip data
	ozz::animation::Animation anim;
	//method to calculate curves given a time point
public:
	AnimationAsset(const std::string& name){}
};

class AnimationState : public IAnimGraphable{
	locked_hashmap<Ref<AnimationAsset>,float, SpinLock> influence;
public:
	/**
	 Add an AnimationAsset to the collection, or change the influence for the existing asset
	 @param inf the influence for this clip
	 */
	void SetAnimationInfluence(Ref<AnimationAsset> asset, float inf = 1){
		influence[asset] = inf;
	}
	
	/**
	 Remove an animation from the collection
	 @param asset the asset to remove
	 */
	void RemoveAnimation(Ref<AnimationAsset> asset){
		influence.erase(asset);
	}
};
}
