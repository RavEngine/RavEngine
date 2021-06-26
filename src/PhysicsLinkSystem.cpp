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

void PhysicsLinkSystemRead::Tick(float fpsScale, const Ref<RigidBodyDynamicComponent> c, Ref<Transform> tr) {
        //physx requires reads and writes to be sequential
        
        //if there is a crash here: dynamicsWorld was not set on this class in the World when it was created
        auto rigid = c.get();
        auto transform = tr.get();
        dynamicsWorld->lockRead();
        auto pos = rigid->getPos();
        auto rot = rigid->getRot();
        dynamicsWorld->unlockRead();
        transform->SetWorldPosition(pos);
        transform->SetWorldRotation(rot);
}

void PhysicsLinkSystemWrite::Tick(float fpsScale, Ref<PhysicsBodyComponent> c, const Ref<Transform> tr) {

        //physx requires reads and writes to be sequential

        //if there is a crash here: dynamicsWorld was not set on this class in the World when it was created
        auto transform = tr.get();
        auto pos = transform->GetWorldPosition();
        auto rot = transform->GetWorldRotation();
        auto rigid = c.get();
        dynamicsWorld->lockWrite();
        rigid->setPos(pos);
        rigid->setRot(rot);
        dynamicsWorld->unlockWrite();
}

const System::list_type PhysicsLinkSystemRead::runbefore = {CTTI<ScriptSystem>()};
