//
//  PhysicsLinkSystem.cpp
//  RavEngine_Static
//
//  Copyright © 2020 Ravbug.
//

#include "PhysicsLinkSystem.hpp"
#include "PhysicsBodyComponent.hpp"
#include "Entity.hpp"
using namespace RavEngine;

void PhysicsLinkSystemRead::operator()(float fpsScale, const RigidBodyDynamicComponent& rigid, Transform& transform) const{
        //physx requires reads and writes to be sequential
        
        //if there is a crash here: dynamicsWorld was not set on this class in the World when it was created
        dynamicsWorld->lockRead();
        auto pos = rigid.getPos();
        auto rot = rigid.getRot();
        dynamicsWorld->unlockRead();
        transform.SetWorldPosition(pos);
        transform.SetWorldRotation(rot);
}

void PhysicsLinkSystemWrite::operator()(float fpsScale, const std::vector<PhysicsBodyComponent*>& r, const std::vector<Transform*>& t) const{

        //physx requires reads and writes to be sequential
        auto& rigid = *(r[0]);
        auto& transform = *(t[0]);

        //if there is a crash here: dynamicsWorld was not set on this class in the World when it was created
        auto pos = transform.GetWorldPosition();
        auto rot = transform.GetWorldRotation();
        dynamicsWorld->lockWrite();
        rigid.setPos(pos);
        rigid.setRot(rot);
        dynamicsWorld->unlockWrite();
}

const System::list_type PhysicsLinkSystemRead::runbefore = {CTTI<ScriptSystem>()};
