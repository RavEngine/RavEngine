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

void PhysicsLinkSystemRead::Tick(float fpsScale, Ref<Component> c, ctti_t id) {
    auto e = c->getOwner().lock();
    if (e){
        //physx requires reads and writes to be sequential
        
        //if there is a crash here: dynamicsWorld was not set on this class in the World when it was created
        auto rigid = std::static_pointer_cast<RigidBodyDynamicComponent>(c);
        auto transform = e->transform();
        dynamicsWorld->lockRead();
        auto pos = rigid->getPos();
        auto rot = rigid->getRot();
        dynamicsWorld->unlockRead();
        transform->SetWorldPosition(pos);
        transform->SetWorldRotation(rot);
    }
   
}

void PhysicsLinkSystemWrite::Tick(float fpsScale, Ref<Component> c, ctti_t id) {
    auto e = c->getOwner().lock();
    if (e){
        //physx requires reads and writes to be sequential

        //if there is a crash here: dynamicsWorld was not set on this class in the World when it was created
        auto transform = e->transform();
        auto pos = transform->GetWorldPosition();
        auto rot = transform->GetWorldRotation();
        auto rigid = std::static_pointer_cast<PhysicsBodyComponent>(c);
        dynamicsWorld->lockWrite();
        rigid->setPos(pos);
        rigid->setRot(rot);
        dynamicsWorld->unlockWrite();
    }
    
}

const System::list_type PhysicsLinkSystemRead::runbefore = {CTTI<ScriptSystem>()};
