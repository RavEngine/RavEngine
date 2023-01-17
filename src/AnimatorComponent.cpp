#include "AnimatorComponent.hpp"
#include <ozz/base/maths/simd_math.h>
#include <ozz/options/options.h>
#include <ozz/animation/runtime/blending_job.h>
#include <ozz/animation/runtime/local_to_model_job.h>
#include "Debug.hpp"

using namespace RavEngine;
using namespace std;

inline float distance(const clamped_vec2& p1, const clamped_vec2& p2){
	return std::sqrt(std::pow(p2.get_x() - p1.get_x(), 2) + std::pow(p2.get_y() - p1.get_y(), 2));
}

void AnimatorComponent::Tick(float timeScale){
	//skip calculation 
	if(!isPlaying){
		return;
	}
	
	auto currentTime = GetApp()->GetCurrentTime();
	//if isBlending, need to calculate both states, and blend between them
	if (isBlending){
		
		auto& fromState = states[stateBlend.from];
		auto& toState = states[stateBlend.to];
			
		//advance playheads
		if (isPlaying){
			//update the tween
			currentBlendingValue = stateBlend.currentTween.step((float)timeScale / stateBlend.currentTween.duration());
		}
		
        auto& cref = *cache;
		fromState.clip->Sample(currentTime, std::max(lastPlayTime,fromState.lastPlayTime), fromState.speed, fromState.isLooping, transforms, cref, skeleton->GetSkeleton().get());
		bool toDone = toState.clip->Sample(currentTime, std::max(lastPlayTime,toState.lastPlayTime), toState.speed, toState.isLooping, transformsSecondaryBlending, cref, skeleton->GetSkeleton().get());
		
		//blend into output
		ozz::animation::BlendingJob::Layer layers[2];
		
		//populate layers
		layers[0].transform = ozz::make_span(transforms);
		layers[0].weight = 1 - currentBlendingValue;
		layers[1].transform = ozz::make_span(transformsSecondaryBlending);
		layers[1].weight = currentBlendingValue;
		
		//when the tween is finished, isBlending = false
		if (stateBlend.currentTween.progress() >= 1.0){
			isBlending = false;
			if (toDone) {
				EndState(toState,stateBlend.from);
			}
		}
		
		ozz::animation::BlendingJob blend_job;
		blend_job.threshold = 0.1f;			//TODO: make threshold configurable
		blend_job.layers = layers;
		blend_job.rest_pose = skeleton->GetSkeleton()->joint_rest_poses();
		
		blend_job.output = make_span(transforms);
		if (!blend_job.Run()) {
			Debug::Fatal("Blend job failed");
		}
	}
	else{
        if (states.contains(currentState)){
            auto& state = states[currentState];
            auto& cref = *cache;
			if (state.clip->Sample(currentTime, std::max(lastPlayTime, state.lastPlayTime), state.speed, state.isLooping, transforms, cref, skeleton->GetSkeleton().get())) {
				EndState(state,currentState);
			}
        }
        else{
            //set all to skeleton bind pose
			for(int i = 0; i < transforms.size(); i++){
				transforms[i] = skeleton->GetSkeleton()->joint_rest_poses()[i];
			}
        }
	}
	
	//convert from local space to model space
	ozz::animation::LocalToModelJob job;
	job.skeleton = skeleton->GetSkeleton().get();
	job.input = ozz::make_span(transforms);
	job.output = ozz::make_span(models);
	
	if (!job.Run()){
		Debug::Fatal("local to model job failed");
	}
	
	// create pose-bindpose skinning matrices
	auto& pose = GetLocalPose();
	auto& bindpose = skeleton->GetBindposes();
	for(int i = 0; i < skinningmats.size(); i++){
		skinningmats[i] = pose[i] * matrix4(bindpose[i]);
	}

	// update world poses
	GetPose();
}

void AnimatorComponent::UpdateSocket(const std::string& name, Transform& t) const{
	for (int i = 0; i < skeleton->GetSkeleton()->num_joints(); i++) {
		auto name = skeleton->GetSkeleton()->joint_names()[i];
		//TODO: set matrix directly instead of with decompose?
        auto& mat = glm_pose[i];

        auto translate = mat[3];
        auto rotation = glm::quat_cast(mat);
		
		t.SetWorldPosition(translate);
		t.SetWorldRotation(rotation);
	}
}

bool AnimBlendTree::Node::Sample(float t, float start, float speed, bool looping, ozz::vector<ozz::math::SoaTransform> &output, ozz::animation::SamplingJob::Context &cache, const ozz::animation::Skeleton* skeleton) const{
	return state->Sample(t, start, speed, looping, output, cache, skeleton);
}

bool AnimBlendTree::Sample(float t, float start, float speed, bool looping, ozz::vector<ozz::math::SoaTransform> &output, ozz::animation::SamplingJob::Context &cache, const ozz::animation::Skeleton* skeleton) const{
	//iterate though the nodes, sample all, and blend
	//calculate the subtracks
	ozz::animation::BlendingJob::Layer layers[kmax_nodes];
	Debug::Assert(states.size() <= kmax_nodes, "An AnimBlendTree can have a maximum of {} nodes",kmax_nodes);
	//stackarray(layers, ozz::animation::BlendingJob::Layer, states.size());
	int index = 0;
	for(auto& row : states){
		Sampler& sampler = const_cast<Sampler&>(row.second);	//TODO: avoid const_cast
		
		//make sure the buffers are the correct size
		if (sampler.locals.size() != skeleton->num_soa_joints()){
			sampler.locals.resize(skeleton->num_soa_joints());
		}

		row.second.node.Sample(t, start, speed, looping, sampler.locals,cache,skeleton);
		
		//populate layers
		layers[index].transform = ozz::make_span(sampler.locals);
		//the influence is calculated as 1 - (distance from control point)
		layers[index].weight = 1.0 - distance(blend_pos, sampler.node.graph_pos) * sampler.node.max_influence;
		index++;
	}
	
	ozz::animation::BlendingJob blend_job;
	blend_job.threshold = 0.1;			//TODO: make threshold configurable
	blend_job.layers = ozz::span(layers,states.size());
	blend_job.rest_pose = skeleton->joint_rest_poses();
	blend_job.output = make_span(output);
	
	if (!blend_job.Run()){
		Debug::Fatal("Blend job failed");
	}

	// TODO: proper end detection for trees
	return false;
}

void AnimatorComponent::DebugDraw(RavEngine::DebugDrawer &dbg, const Transform&) const{
    auto& pose = GetPose();
    for (const auto& p : pose) {
        dbg.DrawSphere(p, debug_color, 0.1);
    }
}
