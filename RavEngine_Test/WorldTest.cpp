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
#include "PhysicsBodyComponent.hpp"
#include "StaticMesh.hpp"

using namespace RavEngine;
Ref<RavEngine::Entity> anonymous;
Ref<RavEngine::Entity> anonymousChild;
Ref<RavEngine::Entity> floorplane;

void ExternalMove(float f) {
    Ref<TestWorld>(GameplayStatics::currentWorld)->player->MoveForward(f);
}

void ExternalMoveRight(float f) {
    Ref<TestWorld>(GameplayStatics::currentWorld)->player->MoveRight(f);
}

void ExternalMoveUp(float f) {
    Ref<TestWorld>(GameplayStatics::currentWorld)->player->MoveUp(f);
}

void ExternalLookUp(float f) {
    Ref<TestWorld>(GameplayStatics::currentWorld)->player->LookUp(f);
}

void ExternalLookRight(float f) {
    Ref<TestWorld>(GameplayStatics::currentWorld)->player->LookRight(f);
}

void SpawnEntities(float f) {
    if (f > 0.99) {
        Ref<TestWorld>(GameplayStatics::currentWorld)->Spawn(new TestEntity());
    }
}

void TestWorld::posttick(float fpsScale){
    auto pos = player->cameraEntity->transform()->GetWorldPosition();
    auto rot = glm::eulerAngles(player->cameraEntity->transform()->GetWorldRotation()); 
    //bgfx::dbgTextPrintf(0, 7, 0x4f, "position (%f, %f, %f)", pos.x, pos.y, pos.z);
    //bgfx::dbgTextPrintf(0, 8, 0x4f, "rotation (%f, %f, %f)", rot.x, rot.y, rot.z);

    auto rotation = quaternion(vector3(0, 0, 0.01 * fpsScale));
    anonymous->transform()->LocalRotateDelta(rotation);
    //anonymous->transform()->LocalScaleDelta(vector3(0.01 * fpsScale,0,0));
}

TestWorld::TestWorld() : World() {
    //spawn player (it will make its camera active)
    Spawn(player);

    player->transform()->LocalTranslateDelta(vector3(0,0,8));

    anonymous = new RavEngine::Entity();
    anonymous->AddComponent<StaticMesh>(new StaticMesh());
    Spawn(anonymous);
    anonymous->transform()->LocalTranslateDelta(vector3(0, 1, 0));

    anonymousChild = new RavEngine::Entity();
    anonymousChild->AddComponent<StaticMesh>(new StaticMesh());
    anonymous->transform()->AddChild(anonymousChild->transform());
    anonymousChild->transform()->LocalTranslateDelta(vector3(3,0,0));
    Spawn(anonymousChild);

    //register the systems that are allowed to run in this World
    RegisterSystem(Ref<Skate>(new Skate()));
    
    Ref<PhysicsLinkSystemRead> plsr = new PhysicsLinkSystemRead();
    RegisterSystem(plsr);

    Ref<PhysicsLinkSystemWrite> plsw = new PhysicsLinkSystemWrite();
    RegisterSystem(plsw);



    //dynamics world must be set in these so that locks can be managed correctly
    plsr->dynamicsWorld = Solver->scene;
    plsw->dynamicsWorld = Solver->scene;

    floorplane = new RavEngine::Entity();
    floorplane->AddComponent<StaticMesh>(new StaticMesh());
    floorplane->transform()->LocalScaleDelta(vector3(10, 0.5, 10));
    floorplane->transform()->LocalTranslateDelta(vector3(0, -20, 0));
    auto mat = PhysicsSolver::phys->createMaterial(0.5, 0.5, 0.5);
    floorplane->AddComponent<RigidBodyStaticComponent>(new RigidBodyStaticComponent());
    floorplane->AddComponent<BoxCollider>(new BoxCollider(vector3(10, 0.5, 10), mat));
    //floorplane->AddSystem<PhysicsLinkSystemRead>();
    floorplane->AddSystem<PhysicsLinkSystemWrite>();
    Spawn(floorplane);

    //setup inputs
    Ref<RavEngine::InputSystem> is = new RavEngine::InputSystem();
    //setup control mappings
    is->AddAxisMap("MoveForward", SDL_SCANCODE_W);
    is->AddAxisMap("MoveForward", SDL_SCANCODE_S, -1);   //go backwards
    is->AddAxisMap("MoveRight", SDL_SCANCODE_A, -1);   //go left
    is->AddAxisMap("MoveRight", SDL_SCANCODE_D);   //go left
    is->AddAxisMap("MoveUp", SDL_SCANCODE_SPACE);
    is->AddAxisMap("MoveUp", SDL_SCANCODE_LSHIFT, -1);
    is->AddAxisMap("LookUp", Special::MOUSEMOVE_YVEL,-1);   //turn up
    is->AddAxisMap("LookRight", Special::MOUSEMOVE_XVEL,-1);
    is->AddAxisMap("SpawnTest", SDL_SCANCODE_G);		//press g to spawn objects
    //bind controls
    is->BindAxis("MoveForward", ExternalMove);
    is->BindAxis("MoveRight", ExternalMoveRight);
    is->BindAxis("MoveUp", ExternalMoveUp);
    is->BindAxis("LookUp", ExternalLookUp);
    is->BindAxis("LookRight", ExternalLookRight);

    is->BindAxis("SpawnTest", SpawnEntities);
    //is->BindAction("Click", click, ActionState::Released);
    RavEngine::GameplayStatics::inputManager = is;
};
