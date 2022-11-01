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
    
    //if there is a crash here: dynamicsWorld was not set on this class in the World when it was created
    rigid.getScene()->lockRead();
    auto pos = rigid.getDynamicsWorldPos();
    auto rot = rigid.getDynamicsWorldRot();
    rigid.getScene()->unlockRead();
    transform.SetWorldPosition(pos);
    transform.SetWorldRotation(rot);
}

void PhysicsLinkSystemWrite::operator()(float fpsScale, const PolymorphicGetResult<PhysicsBodyComponent,World::PolymorphicIndirection>& r, const PolymorphicGetResult<Transform,World::PolymorphicIndirection>& t) const{

    //physx requires reads and writes to be sequential
    auto& rigid = r[0];
    auto& transform = t[0];
    
    if (transform.getTickDirty())
    {
        //if there is a crash here: dynamicsWorld was not set on this class in the World when it was created
        auto pos = transform.GetWorldPosition();
        auto rot = transform.GetWorldRotation();
        rigid.getScene()->lockWrite();
        rigid.setDynamicsWorldPos(pos);
        rigid.setDynamicsWorldRot(rot);
        rigid.getScene()->unlockWrite();
    }
}
