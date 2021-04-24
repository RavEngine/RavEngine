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
#include "App.hpp"

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
		void Sample(float t, ozz::vector<ozz::math::SoaTransform>&, ozz::animation::SamplingCache& cache, const ozz::animation::Skeleton* skeleton) const override;
	};

	static constexpr uint16_t kmax_nodes = 64;
	
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
	void Sample(float t, ozz::vector<ozz::math::SoaTransform>&, ozz::animation::SamplingCache& cache, const ozz::animation::Skeleton* skeleton) const override;
	
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
		float time = 0, speed = 0.001;
		bool isLooping = true;
		
		struct Transition{
			enum class TimeMode{
				Blended = 0,	//the time from this state carries over to the target state
				BeginNew = 1	//the target state's time is set to 0 when the transition begins
			} type;
			tweeny::tween<float> transition;
		};
		
		//transitions out of this state, keyed by ID
		phmap::flat_hash_map<decltype(ID),Transition> exitTransitions;
		
		inline void Tick(float timeScale){
			time += speed * timeScale;
			time = isLooping ? fmod(time,1.0f) : std::min(time,1.0f);
		}
		
		template<typename T>
		inline void SetTransition(decltype(ID) id, T interpolation, float duration, Transition::TimeMode mode = Transition::TimeMode::Blended){
			auto tween = tweeny::from(0.0f).to(1.0f).during(duration * App::evalNormal).via(interpolation);
			exitTransitions[id].transition = tween;
			exitTransitions[id].type = mode;
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
	
	inline decltype(skeleton) GetSkeleton() const{
		return skeleton;
	}
	
protected:
	locked_node_hashmap<id_t,State> states;
	
	struct StateBlend{
		id_t from, to;
		decltype(State::Transition::transition) currentTween;
	} stateBlend;
		
	id_t currentState = 0;
	
	ozz::vector<ozz::math::SoaTransform> transforms, transformsSecondaryBlending;
	ozz::animation::SamplingCache cache;
	ozz::vector<ozz::math::Float4x4> models;
	ozz::vector<matrix4> glm_pose;
	ozz::vector<matrix4> local_pose;
	
	/**
	 Update buffer sizes for current skeleton
	 */
	void UpdateSkeletonData(Ref<SkeletonAsset> sk){
		skeleton = sk;
		const auto n_joints_soa = skeleton->GetSkeleton()->num_soa_joints();
		transforms.resize(n_joints_soa);
		transformsSecondaryBlending.resize(n_joints_soa);
		
		const auto n_joints = skeleton->GetSkeleton()->num_joints();
		models.resize(n_joints);
		cache.Resize(n_joints);
		glm_pose.resize(n_joints);
		local_pose.resize(n_joints);
	}
	
	bool isPlaying = false;
	bool isBlending = false;
	float currentBlendingValue = 0;
	
public:
	/**
	 Get the current pose of the animation in world space
	 @return vector of matrices representing the world-space transformations of every joint in the skeleton for the current animation frame
	 */
	inline const decltype(glm_pose)& GetPose(){
		decimalType matrix[16];
		auto worldMat = getOwner().lock()->transform()->CalculateWorldMatrix();
		for(int i = 0; i < models.size(); i++){
			auto& t = models[i];
			for(int r = 0; r < 4; r++){
				float result[4];
				std::memcpy(result,t.cols + r,sizeof(t.cols[r]));
				decimalType dresult[4];
				for(int j = 0; j < 4; j++){
					dresult[j] = result[j];
				}
				//_mm_store_ps(result,p.cols[r]);
				std::memcpy(matrix + r*4,dresult,sizeof(dresult));
			}
			glm_pose[i] = worldMat * glm::make_mat4(matrix);
		}
		return glm_pose;
	}
	
	inline const decltype(local_pose)& GetLocalPose(){
		decimalType matrix[16];
		for(int i = 0; i < models.size(); i++){
			auto& t = models[i];
			for(int r = 0; r < 4; r++){
				float result[4];
				std::memcpy(result,t.cols + r,sizeof(t.cols[r]));
				decimalType dresult[4];
				for(int j = 0; j < 4; j++){
					dresult[j] = result[j];
				}
				//_mm_store_ps(result,p.cols[r]);
				std::memcpy(matrix + r*4,dresult,sizeof(dresult));
			}
			local_pose[i] = glm::make_mat4(matrix);
		}
		return local_pose;
	}
};

}
