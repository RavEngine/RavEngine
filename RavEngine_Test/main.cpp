//
//  main.cpp
//  RavEngine_Test
//
//  Copyright © 2020 Ravbug. All rights reserved.
//

#include "WorldTest.hpp"
#include <iostream>

#ifdef _WIN32
	#include <Windows.h>
	#include <Xinput.h>
	#pragma comment(lib, "xinput")
	#pragma comment(lib,"Ws2_32")
#endif
#include "GameplayStatics.hpp"
#include "TestEntity.hpp"

using namespace std;

Ref<TestWorld> world;

void click() {
	cout << "click down!" << endl;
}

void ExternalMove(float f) {
	world->player->MoveForward(f);
}

void ExternalMoveRight(float f) {
	world->player->MoveRight(f);
}

void ExternalMoveUp(float f) {
	world->player->MoveUp(f);
}

void ExternalLookRight(float f) {
	world->player->LookRight(f);
}

void ExternalLookUp(float f) {
	world->player->LookUp(f);
}


void SpawnEntities(float f) {
	if (f > 0.9) {
		for (int i = 0; i < 8; i++) {
			Ref<TestEntity> e = new TestEntity();
			GameplayStatics::currentWorld->Spawn(e);
		}
	}
	
}

int main(int argc, const char * argv[]) {
    
    {
       WeakRef<SharedObject> w;
       {
           Ref<SharedObject> re(new SharedObject());
           w = re;
           assert(w.get() == re.get());
       }
       assert(w.get() == nullptr);
   }
	
	//setup video settings
	GameplayStatics::VideoSettings.vsync = true;
	GameplayStatics::VideoSettings.width = 800;
	GameplayStatics::VideoSettings.height = 480;

	//setup input
    Ref<InputSystem> is = new InputSystem();
	//setup control mappings
	is->AddAxisMap("MoveForward", SDL_SCANCODE_W);
	is->AddAxisMap("MoveForward", SDL_SCANCODE_S,-1);   //go backwards
	is->AddAxisMap("MoveRight", SDL_SCANCODE_A, -1);   //go left
	is->AddAxisMap("MoveRight", SDL_SCANCODE_D);   //go left
	is->AddAxisMap("MoveUp", SDL_SCANCODE_SPACE);
	is->AddAxisMap("MoveUp", SDL_SCANCODE_LSHIFT, -1);
	is->AddAxisMap("LookUp", Special::MOUSEMOVE_YVEL, -1);   //turn up
	is->AddAxisMap("LookRight", Special::MOUSEMOVE_XVEL,-1);
	is->AddAxisMap("SpawnTest", SDL_SCANCODE_G);		//press g to spawn objects
	//bind controls
	is->BindAxis("MoveForward",ExternalMove);
	is->BindAxis("MoveRight",ExternalMoveRight);
	is->BindAxis("MoveUp", ExternalMoveUp);
	is->BindAxis("LookUp", ExternalLookUp);
	is->BindAxis("LookRight", ExternalLookRight);
    
	is->BindAxis("SpawnTest", SpawnEntities);
	is->BindAction("Click", click, ActionState::Released);
	GameplayStatics::inputManager = is;

	//load startup world and launch game
	world = new TestWorld();
	GameplayStatics::StartGame(world, "RavEngine Test");
	
	return 0;
}
