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
#include "Queryable.hpp"

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
	
	/**
	 Sample the animation curves in this tree
	 @param t the time to sample
	 @param output the vector to write the output transforms to
	 @param cache a sampling cache, modified when used
	 */
	void Sample(float t, ozz::vector<ozz::math::SoaTransform>&, ozz::animation::SamplingCache& cache, const ozz::animation::Skeleton& skeleton) const override;
	
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
	
	AnimatorComponent(Ref<SkeletonAsset> sk) : skeleton(sk){}
		
	inline void Play(){
		isPlaying = true;
	}
	
	inline void Pause(){
		isPlaying = false;
	}
	
	inline void SetSpeed(float s){
		speed = s;
	}
	
	inline void Seek(float pos){
		time = std::clamp(pos, 0.0f, 1.0f);
	}
	
	void Tick(float timeScale);
	
	inline void SetBlendTree(Ref<AnimBlendTree> t){
		tree = t;
	}
	
protected:
	Ref<AnimBlendTree> tree;
	ozz::vector<ozz::math::SoaTransform> transforms;
	ozz::animation::SamplingCache cache;
	ozz::vector<ozz::math::Float4x4> models;
	
	bool isPlaying = false;
	float time = 0, speed = 1;
};

}
