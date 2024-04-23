#include "AnimatorComponent.hpp"
#include <ozz/base/maths/simd_math.h>
#include <ozz/options/options.h>
#include <ozz/animation/runtime/blending_job.h>
#include <ozz/animation/runtime/local_to_model_job.h>
#include "Debug.hpp"
#include "Transform.hpp"
#include "SkeletonAsset.hpp"

using namespace RavEngine;
using namespace std;
#include <glm/gtc/type_ptr.hpp>

inline float distance(const clamped_vec2& p1, const clamped_vec2& p2){
	return std::sqrt(std::pow(p2.get_x() - p1.get_x(), 2) + std::pow(p2.get_y() - p1.get_y(), 2));
}


/**
Transitions to the new state. If the current state has a transition to the target state, that transition is played.
Otherwise, the state machine simply jumps to the target state without a transition.
@param newState the state to switch to
*/


/**
Create an AnimatorComponent with a SkeletonAsset
@param sk the skeleton asset
*/

RavEngine::AnimatorComponent::AnimatorComponent(Ref<SkeletonAsset> sk) : isPlaying(false), isBlending(false) {
	UpdateSkeletonData(sk);
}

void RavEngine::AnimatorComponent::Goto(id_t newState, bool skipTransition) {
	auto prevState = currentState;
	if (newState != currentState) {
		states[currentState].DoEnd(newState);
	}
	if (skipTransition || !(states.contains(newState) && states.at(currentState).exitTransitions.contains(newState))) {	//just jump to the new state
		currentState = newState;
	}
	else {
		//want to blend to the new state, so set up the blendingclip
		stateBlend.from = currentState;
		stateBlend.to = newState;

		//copy time or reset time on target?
		auto& ns = states.at(currentState).exitTransitions.at(newState);

		switch (ns.type) {
		case State::Transition::TimeMode::BeginNew:
			states.at(newState).lastPlayTime = GetApp()->GetCurrentTime();
			break;
		default: break;
		}

		//seek tween back to beginning
		stateBlend.currentTween = ns.transition;
		stateBlend.currentTween.seek(0);

		isBlending = true;
		currentState = newState;
	}
	states[currentState].DoBegin(prevState);
}


/**
Begin playing this AnimatorController
@param resetPlayhead true if the time of this animator should be reset (for nonlooping animations), false to resume where paused (for looping animations)
*/

void RavEngine::AnimatorComponent::Play(float resetPlayhead) {
	// need to maintain offset from previous play time
	if (!isPlaying) {
		if (resetPlayhead) {
			lastPlayTime = GetApp()->GetCurrentTime();
		}
		else {
			lastPlayTime = GetApp()->GetCurrentTime() - lastPlayTime;
		}
		isPlaying = true;
	}
}

void RavEngine::AnimatorComponent::Pause() {
	// record pause time so that resume begins in the correct place
	if (isPlaying) {
		lastPlayTime = GetApp()->GetCurrentTime();
	}
	isPlaying = false;
}

void AnimatorComponent::Tick(const Transform& t){
	//skip calculation 
	if(!isPlaying){
		return;
	}
	
    auto timeScale = GetApp()->GetCurrentFPSScale();
    
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
	GetPose(t);
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

/**
Update buffer sizes for current skeleton
*/

inline void RavEngine::AnimatorComponent::UpdateSkeletonData(Ref<SkeletonAsset> sk) {
	skeleton = sk;
	const auto n_joints_soa = skeleton->GetSkeleton()->num_soa_joints();
	transforms.resize(n_joints_soa);
	transformsSecondaryBlending.resize(n_joints_soa);

	const auto n_joints = skeleton->GetSkeleton()->num_joints();
	models.resize(n_joints);
	cache->Resize(n_joints);
	glm_pose.resize(n_joints);
	local_pose.resize(n_joints);
	skinningmats.resize(n_joints);
}


/**
Get the current pose of the animation in world space
@return vector of matrices representing the world-space transformations of every joint in the skeleton for the current animation frame
*/

const decltype(RavEngine::AnimatorComponent::glm_pose)& RavEngine::AnimatorComponent::GetPose(const Transform& t) const {
	decimalType matrix[16]{ 0 };
	auto worldMat = t.GetWorldMatrix();
	for (int i = 0; i < models.size(); i++) {
		auto& t = models[i];
		for (int r = 0; r < 4; r++) {
			float result[4];
			std::memcpy(result, t.cols + r, sizeof(t.cols[r]));
			decimalType dresult[4];
			for (int j = 0; j < 4; j++) {
				dresult[j] = result[j];
			}
			//_mm_store_ps(result,p.cols[r]);
			std::memcpy(matrix + r * 4, dresult, sizeof(dresult));
		}
		glm_pose[i] = worldMat * glm::make_mat4(matrix);
	}
	return glm_pose;
}

const decltype(RavEngine::AnimatorComponent::local_pose)& RavEngine::AnimatorComponent::GetLocalPose() {
	decimalType matrix[16];
	for (int i = 0; i < models.size(); i++) {
		auto& t = models[i];
		for (int r = 0; r < 4; r++) {
			float result[4];
			std::memcpy(result, t.cols + r, sizeof(t.cols[r]));
			decimalType dresult[4];
			for (int j = 0; j < 4; j++) {
				dresult[j] = result[j];
			}
			//_mm_store_ps(result,p.cols[r]);
			std::memcpy(matrix + r * 4, dresult, sizeof(dresult));
		}
		local_pose[i] = glm::make_mat4(matrix);
	}
	return local_pose;
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
#if !RVE_SERVER

void AnimatorComponent::DebugDraw(RavEngine::DebugDrawer &dbg, const Transform& t) const{
    auto& pose = GetPose(t);
    for (const auto& p : pose) {
        dbg.DrawSphere(p, debug_color, 0.1);
    }
}
#endif
