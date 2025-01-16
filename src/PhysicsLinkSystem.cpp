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
#include "World.hpp"
#include "PhysicsSolver.hpp"

using namespace RavEngine;

void PhysicsLinkSystemRead::operator()(const RigidBodyDynamicComponent& rigid, Transform& transform) const{
    //physx requires reads and writes to be sequential
    if (rigid.IsSleeping() && !rigid.setPoseNeedsSync){
        return;
    }
    rigid.setPoseNeedsSync = false;
    auto pose = rigid.getDynamicsWorldPose();
    transform.SetWorldPosition(pose.first);
    transform.SetWorldRotation(pose.second);
}

void RavEngine::PhysicsLinkSystemWrite::before(World* world) const
{
    world->Solver->scene->lockWrite();
}

void RavEngine::PhysicsLinkSystemWrite::after(World* world) const
{
    world->Solver->scene->unlockWrite();
}

void PhysicsLinkSystemWrite::operator()(const RigidBodyStaticComponent& rigid, const Transform& transform) const{

    //physx requires reads and writes to be sequential
    auto pos = transform.GetWorldPosition();
    auto rot = transform.GetWorldRotation();
    rigid.setDynamicsWorldPoseNoLock(pos,rot);
}
