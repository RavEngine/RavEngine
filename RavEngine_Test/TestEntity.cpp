//
//  TestEntity.cpp
//  RavEngine_Test
//
//  Copyright © 2020 Ravbug. All rights reserved.
//

#include "TestEntity.hpp"
#include "SkateSystem.h"
#include "PhysicsLinkSystem.hpp"
#include "PhysicsCollider.hpp"
#include <PxMaterial.h>
#include "PhysicsSolver.hpp"
#include <iostream>
#include "StaticMesh.hpp"

using namespace std;
using namespace physx;


TestEntity::TestEntity() : Entity(){
    AddSystem<Ref<PhysicsLinkSystemRead>>();
    //AddSystem<SkateRef>();
    AddSystem<Ref<PhysicsLinkSystemWrite>>();

    //set the filter layers
    auto r = AddComponent<RigidBodyDynamicComponent>(new RigidBodyDynamicComponent(FilterLayers::L0,FilterLayers::L0 | FilterLayers::L1));

    //add a box collision to the PhysX component
    physx::PxReal val(0.5);
    auto mat = PhysicsSolver::phys->createMaterial(val,val,val);
    AddComponent<BoxCollider>(new BoxCollider(vector3(5, 5, 5),mat));

    //default staticmesh
    AddComponent<StaticMesh>(new StaticMesh());
}

void TestEntity::Tick(float scale) {
   /* auto vec = transform()->position.load();
    cout << vec.X << " " << vec.Y << " " << vec.Z << endl;*/
}

void TestEntity::OnColliderEnter(const WeakRef<PhysicsBodyComponent>& other)
{
    cout << this << " collided with " << other.get() << endl;
}
