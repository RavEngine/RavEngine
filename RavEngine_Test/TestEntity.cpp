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
using namespace RavEngine;

Ref<RavEngine::PhysicsMaterial> TestEntity::sharedMat;
Ref<RavEngine::Material> TestEntity::sharedMatInstance;

TestEntity::TestEntity() : Entity(){
    AddSystem<PhysicsLinkSystemRead>();
    //AddSystem<SkateRef>();
    AddSystem<PhysicsLinkSystemWrite>();

    //set the filter layers
    auto r = AddComponent<RigidBodyDynamicComponent>(new RigidBodyDynamicComponent(FilterLayers::L0,FilterLayers::L0 | FilterLayers::L1));

    //add a box collision to the PhysX component
    if (sharedMat.isNull()) {
        sharedMat = new PhysicsMaterial(0.5, 0.5, 0.5);
    }
    AddComponent<BoxCollider>(new BoxCollider(vector3(1, 1, 1),sharedMat));

    //default staticmesh
    auto mesh = AddComponent<StaticMesh>(new StaticMesh());
    if (sharedMatInstance.isNull()) {
        if (MaterialManager::HasMaterialByName("defaultMaterial")) {
            sharedMatInstance = MaterialManager::GetMaterialByName("defaultMaterial");
        }
        else {
            sharedMatInstance = new Material();
        }
    }
    mesh->SetMaterial(sharedMatInstance);
}

void TestEntity::Tick(float scale) {

    //delete entities below y=-30
    if (transform()->GetWorldPosition().y < -30) {
        Destroy();
    }
}

void TestEntity::OnColliderEnter(const WeakRef<PhysicsBodyComponent>& other)
{
}
