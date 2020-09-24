//
//  TestEntity.cpp
//  RavEngine_Test
//
//  Copyright © 2020 Ravbug. All rights reserved.
//

#include "TestEntity.hpp"
#include "SkateSystem.h"
#include "RavEngine/PhysicsLinkSystem.hpp"
#include "RavEngine/PhysicsCollider.hpp"
#include <PxMaterial.h>
#include "RavEngine/PhysicsSolver.hpp"
#include <iostream>
#include "RavEngine/StaticMesh.hpp"

using namespace std;
using namespace physx;
using namespace RavEngine;

Ref<RavEngine::PhysicsMaterial> TestEntity::sharedMat;
Ref<RavEngine::DefaultMaterialInstance> TestEntity::sharedMatInst;
Ref<MeshAsset> TestEntity::sharedMesh;

TestEntity::TestEntity() : Entity(){

    //attach the script
    auto script = AddComponent<TestEntityController>(new TestEntityController());

    //set the filter layers
    auto r = AddComponent<RigidBodyDynamicComponent>(new RigidBodyDynamicComponent(FilterLayers::L0,FilterLayers::L0 | FilterLayers::L1));
    r->AddReceiver(script.get());

    //add a box collision to the PhysX component
    if (sharedMat.isNull()) {
        sharedMat = new PhysicsMaterial(0.5, 0.5, 0.5);
    }
    AddComponent<BoxCollider>(new BoxCollider(vector3(1, 1, 1),sharedMat));
	
	if (sharedMesh.isNull()){
		sharedMesh = new MeshAsset();
	}

    //default staticmesh
    auto mesh = AddComponent<StaticMesh>(new StaticMesh(sharedMesh));
    if (sharedMatInst.isNull()) {
        if (MaterialManager::HasMaterialByName("cubes")) {
            sharedMatInst = new DefaultMaterialInstance(MaterialManager::GetMaterialByName("cubes"));
        }
        else {
            sharedMatInst = new DefaultMaterialInstance(new DefaultMaterial());
        }
    }
    mesh->SetMaterial(sharedMatInst);
}

void TestEntityController::Tick(float scale) {

    //delete entities below y=-30
    if (transform()->GetWorldPosition().y < -30) {
        Destroy();
    }
}

void TestEntityController::OnColliderEnter(const WeakRef<PhysicsBodyComponent>& other)
{
    //cout << "hit" << endl;
}
