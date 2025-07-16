#include "Constraint.hpp"
#include "AnimatorComponent.hpp"
#include "SkeletonAsset.hpp"
#include "Debug.hpp"

using namespace RavEngine;
using namespace std;

void ConstraintTarget::Destroy(){
	auto owner = GetOwner();
	// disconnect the targeters by resetting their handles
	for(const auto entity : targeters){
		auto ds = entity.GetAllComponentsPolymorphic<Constraint>();
		for(uint16_t i = 0; i < ds.size(); i++){
			auto a = ds.HandleFor<PolymorphicComponentHandle<Constraint>>(i);
			a->target.reset();
		}
	}
}

Constraint::Constraint(Entity id, decltype(target) targetEntity) : ComponentWithOwner(id), target(targetEntity){
	Debug::Assert(target, "Cannot add Constraint to invalid entity!");
	target->AddTargeter(GetOwner());;
}

void Constraint::Destroy(){
	if (target){
		target->DeleteTargeter(GetOwner());
	}
}


SocketConstraint::SocketConstraint(Entity id, decltype(target) t, const decltype(boneTarget)& tgt) : Constraint(id,t) , boneTarget(tgt){
	//TODO: check if target entity has the bone
	Debug::Assert(target.GetOwner().GetComponent<AnimatorComponent>().GetSkeleton()->HasBone(tgt), "Cannot add socket constraint to nonexistent bone {}", tgt);
}

void SocketSystem::operator()(DataProvider& dp, const SocketConstraint& constraint, Transform& trns){
	if (constraint){
		auto& animator = constraint.GetTarget()->GetOwner().GetComponent<AnimatorComponent>();
		animator.UpdateSocket(constraint.boneTarget,trns);
	}
}
