//
//  WorldTest.cpp
//  RavEngine_Test
//
//  Copyright © 2020 Ravbug. All rights reserved.
//

#include "WorldTest.hpp"
#include "System.hpp"
#include "SkateSystem.h"
#include "TestEntity.hpp"
#include "GameplayStatics.hpp"
#include "PhysicsLinkSystem.hpp"
#include "StaticMesh.hpp"

using namespace RavEngine;
Ref<RavEngine::Entity> anonymous;

void TestWorld::posttick(float fpsScale){
    auto pos = player->cameraEntity->transform()->GetWorldPosition();
    auto rot = glm::eulerAngles(player->cameraEntity->transform()->GetWorldRotation()); 
    //bgfx::dbgTextPrintf(0, 7, 0x4f, "position (%f, %f, %f)", pos.x, pos.y, pos.z);
    //bgfx::dbgTextPrintf(0, 8, 0x4f, "rotation (%f, %f, %f)", rot.x, rot.y, rot.z);

    auto rotation = quaternion(vector3(0, 0, 0.01));
    anonymous->transform()->LocalRotateDelta(rotation);
}

TestWorld::TestWorld() : World() {
    //spawn player (it will make its camera active)
    Spawn(player);

    player->transform()->LocalTranslateDelta(vector3(0,0,0));

    anonymous = new RavEngine::Entity();
    anonymous->AddComponent<StaticMesh>(new StaticMesh());
    Spawn(anonymous);
    anonymous->transform()->LocalTranslateDelta(vector3(0, 5, 0));

    //register the systems that are allowed to run in this World
    RegisterSystem(new Skate());
    
    Ref<PhysicsLinkSystemRead> plsr = new PhysicsLinkSystemRead();
    RegisterSystem(plsr);

    Ref<PhysicsLinkSystemWrite> plsw = new PhysicsLinkSystemWrite();
    RegisterSystem(plsw);

    //dynamics world must be set in these so that locks can be managed correctly
    plsr->dynamicsWorld = Solver->scene;
    plsw->dynamicsWorld = Solver->scene;
};
