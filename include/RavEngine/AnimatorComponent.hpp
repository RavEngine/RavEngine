#pragma once
#include "Component.hpp"
#include <ozz/animation/runtime/animation.h>
#include <ozz/base/maths/soa_transform.h>
#include "AnimationAsset.hpp"
#include "DataStructures.hpp"
#include "SkeletonAsset.hpp"
#include "Ref.hpp"
#include <algorithm>
#include <vector>
#include "Tween.hpp"

namespace RavEngine{

class normalized_vec2{
	float x, y;
public:
	normalized_vec2(float ix, float iy) : x(std::clamp(ix,-1.0f,1.0f)), y(std::clamp(iy,-1.0f,1.0f)){}
	normalized_vec2() : normalized_vec2(0,0){}
	
	inline decltype(x) get_x() const{
		return x;
	}
	inline decltype(y) get_y() const{
		return y;
	}
};

struct AnimBlendTree : public IAnimGraphable{
	struct Node : public IAnimGraphable{
		Ref<IAnimGraphable> state;
		normalized_vec2 graph_pos;
		float max_influence = 1;
		
		Node(){}
		
		Node(const Node& other) : state(other.state), graph_pos(other.graph_pos), max_influence(other.max_influence){}
		
		template<typename T>
		Node(Ref<T> s, const normalized_vec2& pos, float i = 1) : state(std::static_pointer_cast<IAnimGraphable>(s)), graph_pos(pos), max_influence(i){}
		
		/**
		 Sample the animation curves in this tree
		 @param t the time to sample
		 @param output the vector to write the output transforms to
		 @param cache a sampling cache, modified when used
		 */
		void Sample(float t, ozz::vector<ozz::math::SoaTransform>&, ozz::animation::SamplingCache& cache, const ozz::animation::Skeleton& skeleton) const override;
	};
	
	/**
	 Insert a node into the tree at the id. If a node already exists at that ID, it is replaced.
	 @param id the identifier for the node. Recommended to create an enum to use here.
	 @param node the node to insert into the tree.
	 */
	inline void InsertNode(uint8_t id, const Node& node){
		states[id].node = node;
	}
	
	/**
	 Remove a node given an ID
	 @param id the id to remove
	 */
	inline void DeleteNode(uint8_t id){
		states.erase(id);
	}
	
	/**
	 Get a node reference to make modifications to it
	 @param id the ID to get a node for
	 @returns node reference
	 @throws if no node exists at id
	 */
	Node& getNode(const uint8_t id){
		return states.at(id).node;
	}
	
	inline bool IsEmpty() const{
		return states.empty();
	}
	
	inline void Clear(){
		states.clear();
	}
	
	/**
	 Sample the animation curves in this tree
	 @param t the time to sample
	 @param output the vector to write the output transforms to
	 @param cache a sampling cache, modified when used
	 */
	void Sample(float t, ozz::vector<ozz::math::SoaTransform>&, ozz::animation::SamplingCache& cache, const ozz::animation::Skeleton& skeleton) const override;
	
	inline void SetBlendPos(const normalized_vec2& newPos){
		blend_pos = newPos;
	}
	
private:
	struct Sampler{
		ozz::vector<ozz::math::SoaTransform> locals;
		Node node;
	};
	locked_node_hashmap<uint8_t,Sampler,SpinLock> states;
	normalized_vec2 blend_pos;
};

class AnimatorComponent : public Component, public Queryable<AnimatorComponent>{
protected:
	
	Ref<SkeletonAsset> skeleton;
public:
	
	//a node in the state machine
	struct State{
		unsigned short ID;
		Ref<IAnimGraphable> clip;
		float time = 0, speed = 0.1;
		bool isLooping = true;
		
		struct Transition{
			enum class TimeMode{
				Blended = 0,	//the time from this state carries over to the target state
				BeginNew = 1	//the target state's time is set to 0 when the transition begins
			} type;
			Tween<float> transition;
		};
		
		//transitions out of this state, keyed by ID
		phmap::flat_hash_map<decltype(ID),Transition> exitTransitions;
		float currentBlendingValue = 0;
		
		inline void Tick(float timeScale){
			time += speed * timeScale;
			time = isLooping ? fmod(time,1.0f) : std::min(time,1.0f);
		}
		
		template<typename T>
		inline void SetTransition(decltype(ID) id, T interpolation, float duration){
			auto tween = Tween<float>([this](float val){
				currentBlendingValue = val;
			},0.0);
			tween.AddKeyframe(duration,interpolation,1.0);
			exitTransitions[id].transition = tween;
		}
	};
	
	typedef decltype(State::ID) id_t;
	
	/**
	 Create an AnimatorComponent with a SkeletonAsset
	 @param sk the skeleton asset
	 */
	AnimatorComponent(Ref<SkeletonAsset> sk){
		UpdateSkeletonData(sk);
	}
		
	/**
	 Transitions to the new state. If the current state has a transition to the target state, that transition is played.
	 Otherwise, the state machine simply jumps to the target state without a transition.
	 @param newState the state to switch to
	 */
	inline void Goto(id_t newState, bool skipTransition = false){
		if (skipTransition || !(states.contains(newState) && states.at(currentState).exitTransitions.contains(newState))){	//just jump to the new state
			currentState = newState;
		}
		else{
			//want to blend to the new state, so set up the blendingclip
			stateBlend.from = currentState;
			stateBlend.to = newState;
			
			//copy time or reset time on target?
			auto& ns = states.at(newState);
			switch(ns.exitTransitions[newState].type){
				case State::Transition::TimeMode::Blended:
					ns.time = states.at(currentState).time;
					break;
				case State::Transition::TimeMode::BeginNew:
					ns.time = 0;
					break;
			}
			
			//seek tween back to beginning
			stateBlend.currentTween = states.at(currentState).exitTransitions.at(newState).transition;
			stateBlend.currentTween.seek(0);
			
			isBlending = true;
			currentState = newState;
		}
	}
	
	/**
	 Add a state to the state machine
	 @param state the state to insert
	 */
	inline void InsertState(const State& state){
		states.insert(std::make_pair(state.ID,state));
	}
	
	inline void Play(){
		isPlaying = true;
	}
	
	inline void Pause(){
		isPlaying = false;
	}

	void Tick(float timeScale);	
	
protected:
	locked_node_hashmap<id_t,State> states;
	
	struct StateBlend{
		id_t from, to;
		decltype(State::Transition::transition) currentTween;
	} stateBlend;
	
	
	id_t currentState;
	
	ozz::vector<ozz::math::SoaTransform> transforms, transformsSecondaryBlending;
	ozz::animation::SamplingCache cache;
	ozz::vector<ozz::math::Float4x4> models;
	
	/**
	 Update buffer sizes for current skeleton
	 */
	void UpdateSkeletonData(Ref<SkeletonAsset> sk){
		skeleton = sk;
		transforms.resize(skeleton->GetSkeleton().num_soa_joints());
		transformsSecondaryBlending.resize(skeleton->GetSkeleton().num_soa_joints());
		models.resize(skeleton->GetSkeleton().num_joints());
		cache.Resize(skeleton->GetSkeleton().num_joints());
	}
	
	bool isPlaying = false;
	bool isBlending = false;
};

}
