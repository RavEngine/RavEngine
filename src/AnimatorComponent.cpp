#include "AnimatorComponent.hpp"
#include <ozz/base/maths/simd_math.h>
#include <ozz/options/options.h>
#include <ozz/animation/runtime/blending_job.h>
#include <ozz/animation/runtime/local_to_model_job.h>
#include "Debug.hpp"

using namespace RavEngine;

inline float distance(const normalized_vec2& p1, const normalized_vec2& p2){
	return std::sqrt(std::pow(p2.get_x() - p1.get_x(), 2) + std::pow(p2.get_y() - p1.get_y(), 2));
}

void AnimatorComponent::Tick(float timeScale){
	//calculate the tree
	tree->Sample(time, transforms, cache,skeleton->GetSkeleton());
	
	//TODO: advance the time
	
	//convert from local space to model space
	ozz::animation::LocalToModelJob job;
	job.skeleton = &skeleton->GetSkeleton();
	job.input = ozz::make_span(transforms);
	job.output = ozz::make_span(models);
	
	if (!job.Run()){
		Debug::Fatal("local to model job failed");
	}
}

void AnimBlendTree::Node::Sample(float t, ozz::vector<ozz::math::SoaTransform> &output, ozz::animation::SamplingCache &cache, const ozz::animation::Skeleton& skeleton) const{
	state->Sample(t, output, cache, skeleton);
}

void AnimBlendTree::Sample(float t, ozz::vector<ozz::math::SoaTransform> &output, ozz::animation::SamplingCache &cache, const ozz::animation::Skeleton& skeleton) const{
	//iterate though the nodes, sample all, and blend
	//calculate the subtracks
	stackarray(layers, ozz::animation::BlendingJob::Layer, states.size());
	int index = 0;
	for(auto& row : states){
		Sampler& sampler = const_cast<Sampler&>(row.second);	//TODO: avoid const_cast
		row.second.node.Sample(t,sampler.locals,cache,skeleton);
		
		//populate layers
		layers[index].transform = ozz::make_span(sampler.locals);
		//the influence is calculated as 1 - (distance from control point)
		layers[index].weight = 1.0 - distance(blend_pos, sampler.node.graph_pos) * sampler.node.max_influence;
		index++;
	}
	
	ozz::animation::BlendingJob blend_job;
	blend_job.threshold = 0;			//TODO: make threshold configurable
	blend_job.layers = ozz::span(layers,states.size());
	blend_job.bind_pose = skeleton.joint_bind_poses();
	
	blend_job.output = make_span(output);
	if (!blend_job.Run()){
		Debug::Fatal("Blend job failed");
	}
}
