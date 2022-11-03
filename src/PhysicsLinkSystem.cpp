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

void PhysicsLinkSystemRead::operator()(float fpsScale, const RigidBodyDynamicComponent& rigid, Transform& transform) const{
    //physx requires reads and writes to be sequential
    
    rigid.getScene()->lockRead();
    auto pose = rigid.getDynamicsWorldPose();
    rigid.getScene()->unlockRead();
    transform.SetWorldPosition(pose.first);
    transform.SetWorldRotation(pose.second);
}

void PhysicsLinkSystemWrite::operator()(float fpsScale, const PolymorphicGetResult<PhysicsBodyComponent,World::PolymorphicIndirection>& r, const PolymorphicGetResult<Transform,World::PolymorphicIndirection>& t) const{

    //physx requires reads and writes to be sequential
    auto& rigid = r[0];
    auto& transform = t[0];
    
    auto pos = transform.GetWorldPosition();
    auto rot = transform.GetWorldRotation();
    rigid.getScene()->lockWrite();
    rigid.setDynamicsWorldPose(pos,rot);
    rigid.getScene()->unlockWrite();
}
