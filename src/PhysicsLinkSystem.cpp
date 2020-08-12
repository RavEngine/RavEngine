//
//  PhysicsLinkSystem.cpp
//  RavEngine_Static
//
//  Copyright © 2020 Ravbug. All rights reserved.
//

#include "PhysicsLinkSystem.hpp"
#include "PhysicsBodyComponent.hpp"
using namespace RavEngine;

void PhysicsLinkSystemRead::Tick(float fpsScale, Ref<Entity> e) const{
    //physx requires reads and writes to be sequential

    //if there is a crash here: dynamicsWorld was not set on this class in the World when it was created
    auto rigid = e->Components().GetComponentOfSubclass<PhysicsBodyComponent>();
    dynamicsWorld->lockRead();
    auto pos = rigid->getPos();
    auto rot = rigid->getRot();
    dynamicsWorld->unlockRead();
    e->transform()->SetWorldPosition(pos);
    e->transform()->SetWorldRotation(rot);
}

void PhysicsLinkSystemWrite::Tick(float fpsScale, Ref<Entity> e) const{
    //physx requires reads and writes to be sequential

    //if there is a crash here: dynamicsWorld was not set on this class in the World when it was created
    auto transform = e->transform();
    auto pos = transform->GetWorldPosition();
    auto rot = transform->GetWorldRotation();
    auto rigid = e->Components().GetComponentOfSubclass<PhysicsBodyComponent>();
    dynamicsWorld->lockWrite();
    rigid->setPos(pos);
    rigid->setRot(rot);
    dynamicsWorld->unlockWrite();
}
