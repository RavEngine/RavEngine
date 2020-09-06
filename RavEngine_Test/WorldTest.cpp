//
//  WorldTest.cpp
//  RavEngine_Test
//
//  Copyright © 2020 Ravbug. All rights reserved.
//

#include "WorldTest.hpp"
#include "RavEngine/System.hpp"
#include "SkateSystem.h"
#include "TestEntity.hpp"
#include "RavEngine/GameplayStatics.hpp"
#include "RavEngine/PhysicsLinkSystem.hpp"
#include "RavEngine/PhysicsBodyComponent.hpp"
#include "RavEngine/StaticMesh.hpp"
#include "RavEngine/PhysicsMaterial.hpp"

using namespace RavEngine;
using namespace std;
Ref<RavEngine::Entity> anonymous;
Ref<RavEngine::Entity> anonymousChild;
Ref<RavEngine::Entity> floorplane;

void TestWorld::SpawnEntities(float f) {
    if (f > 0.99) {
       Spawn(new TestEntity());
    }
}

void TestWorld::ResetCam() {
	player->transform()->SetWorldPosition(vector3(0, -10, 50));
	player->transform()->SetWorldRotation(quaternion());
}

void TestWorld::posttick(float fpsScale){
    auto pos = player->cameraEntity->transform()->GetWorldPosition();
    auto rot = glm::eulerAngles(player->cameraEntity->transform()->GetWorldRotation()); 
    //bgfx::dbgTextPrintf(0, 7, 0x4f, "position (%f, %f, %f)", pos.x, pos.y, pos.z);
    //bgfx::dbgTextPrintf(0, 8, 0x4f, "rotation (%f, %f, %f)", rot.x, rot.y, rot.z);

    auto rotation = quaternion(vector3(0, 0, 0.01 * fpsScale));
    anonymous->transform()->LocalRotateDelta(rotation);
    //anonymous->transform()->LocalScaleDelta(vector3(0.01 * fpsScale,0,0));
    scale = fpsScale;
}

TestWorld::TestWorld() : World() {
    //weak ref test
    {
        WeakRef<SharedObject> weak;
        {
            Ref<SharedObject> obj(new SharedObject());
            weak = obj;
        }
        assert(weak.isNull());
    }


	//setup inputs
	Ref<RavEngine::InputManager> is = new RavEngine::InputManager();
	//setup control mappings
	is->AddAxisMap("MoveForward", SDL_SCANCODE_W);
	is->AddAxisMap("MoveForward", SDL_SCANCODE_S, -1);  //go backwards
	is->AddAxisMap("MoveRight", SDL_SCANCODE_A, -1);    //go left
	is->AddAxisMap("MoveRight", SDL_SCANCODE_D);        //turn left
	is->AddAxisMap("MoveUp", SDL_SCANCODE_SPACE);
	is->AddAxisMap("MoveUp", SDL_SCANCODE_LSHIFT, -1);
	is->AddAxisMap("LookUp", Special::MOUSEMOVE_YVEL,-1);   //turn up
	is->AddAxisMap("LookRight", Special::MOUSEMOVE_XVEL,-1);
	is->AddAxisMap("SpawnTest", SDL_SCANCODE_G);		//press g to spawn objects
	is->AddActionMap("ResetCam", SDL_SCANCODE_R);
	is->AddActionMap("SampleFPS",SDL_SCANCODE_T);

    //game controller input
    is->AddAxisMap("MoveForward", ControllerAxis::SDL_CONTROLLER_AXIS_LEFTY, -1);
    is->AddAxisMap("MoveRight", ControllerAxis::SDL_CONTROLLER_AXIS_LEFTX);
    is->AddAxisMap("LookRight", ControllerAxis::SDL_CONTROLLER_AXIS_RIGHTX, -10);
    is->AddAxisMap("LookUp", ControllerAxis::SDL_CONTROLLER_AXIS_RIGHTY, -10);
    is->AddAxisMap("SpawnTest", ControllerButton::SDL_CONTROLLER_BUTTON_A);
    is->AddActionMap("ResetCam", ControllerButton::SDL_CONTROLLER_BUTTON_START);
    is->AddActionMap("SampleFPS", ControllerButton::SDL_CONTROLLER_BUTTON_Y);


	//bind controls
	is->BindAxis("MoveForward", player.get(), &PlayerActor::MoveForward);
	is->BindAxis("MoveRight", player.get(), &PlayerActor::MoveRight);
	is->BindAxis("MoveUp", player.get(),&PlayerActor::MoveUp);
	is->BindAxis("LookUp", player.get(),&PlayerActor::LookUp);
	is->BindAxis("LookRight", player.get(), &PlayerActor::LookRight);
	
	is->BindAxis("SpawnTest", this, &TestWorld::SpawnEntities);
	is->BindAction("ResetCam", this, &TestWorld::ResetCam, ActionState::Pressed);
	
    //test unbinding
	is->UnbindAxis("SpawnTest", this, &TestWorld::SpawnEntities);
	is->UnbindAction("ResetCam", this, &TestWorld::ResetCam, ActionState::Pressed);

    is->BindAxis("SpawnTest", this, &TestWorld::SpawnEntities);
    is->BindAction("ResetCam", this, &TestWorld::ResetCam, ActionState::Pressed);
	is->BindAction("SampleFPS",this, &TestWorld::SampleFPS,ActionState::Pressed);
	//is->BindAction("Click", click, ActionState::Released);
	RavEngine::GameplayStatics::inputManager = is;
	
    //spawn player (it will make its camera active)
    Spawn(player);

    player->transform()->LocalTranslateDelta(vector3(0,-10,50));

    auto material = new Material();

    anonymous = new RavEngine::Entity();
    anonymous->AddComponent<StaticMesh>(new StaticMesh())->SetMaterial(material);
    Spawn(anonymous);
    anonymous->transform()->LocalTranslateDelta(vector3(0, 1, 0));

    anonymousChild = new RavEngine::Entity();
    anonymousChild->AddComponent<StaticMesh>(new StaticMesh())->SetMaterial(material);;
    anonymous->transform()->AddChild(anonymousChild->transform());
    anonymousChild->transform()->LocalTranslateDelta(vector3(3,0,0));
    Spawn(anonymousChild);

    //register the systems that are allowed to run in this World
    //RegisterSystem(Ref<Skate>(new Skate()));

    Ref<PhysicsLinkSystemWrite> plsw = new PhysicsLinkSystemWrite();
    RegisterSystem(plsw);

    Ref<PhysicsLinkSystemRead> plsr = new PhysicsLinkSystemRead();
    RegisterSystem(plsr);

    //dynamics world must be set in these so that locks can be managed correctly
    plsr->dynamicsWorld = Solver->scene;
    plsw->dynamicsWorld = Solver->scene;

    floorplane = new RavEngine::Entity();
    floorplane->AddComponent<StaticMesh>(new StaticMesh())->SetMaterial(material);
    floorplane->transform()->LocalScaleDelta(vector3(10, 0.5, 10));
    floorplane->transform()->LocalTranslateDelta(vector3(0, -20, 0));
    floorplane->AddComponent<RigidBodyStaticComponent>(new RigidBodyStaticComponent());
    floorplane->AddComponent<BoxCollider>(new BoxCollider(vector3(10, 0.5, 10), new PhysicsMaterial(0.5,0.5,0.5)));
    Spawn(floorplane);
};
