//
//  PhysicsLinkSystem.cpp
//  RavEngine_Static
//
//  Copyright © 2020 Ravbug.
//

#include "PhysicsLinkSystem.hpp"
#include "PhysicsBodyComponent.hpp"
#include "Entity.hpp"
#include "Transform.hpp"

using namespace RavEngine;

void PhysicsLinkSystemRead::operator()(const RigidBodyDynamicComponent& rigid, Transform& transform) const{
    //physx requires reads and writes to be sequential
    if (rigid.IsSleeping()){
        return;
    }
    auto pose = rigid.getDynamicsWorldPose();
    transform.SetWorldPosition(pose.first);
    transform.SetWorldRotation(pose.second);
}

void PhysicsLinkSystemWrite::operator()(const RigidBodyStaticComponent& rigid, const Transform& transform) const{

    //physx requires reads and writes to be sequential
    auto pos = transform.GetWorldPosition();
    auto rot = transform.GetWorldRotation();
    rigid.setDynamicsWorldPose(pos,rot);
}
