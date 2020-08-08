 
#include <RavEngine_App.h>
#include <WeakRef.hpp>
#include <SharedObject.hpp>
#include <cassert>
#include <GameplayStatics.hpp>
#include "WorldTest.hpp"

class TestApp : public RavEngine_App{
	
	void OnStartup(int argc, char** argv) override{
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

		//setup inputs
		//setup input
		Ref<InputSystem> is = new InputSystem();
		//setup control mappings
		//is->AddAxisMap("MoveForward", SDL_SCANCODE_W);
		//is->AddAxisMap("MoveForward", SDL_SCANCODE_S, -1);   //go backwards
		//is->AddAxisMap("MoveRight", SDL_SCANCODE_A, -1);   //go left
		//is->AddAxisMap("MoveRight", SDL_SCANCODE_D);   //go left
		//is->AddAxisMap("MoveUp", SDL_SCANCODE_SPACE);
		//is->AddAxisMap("MoveUp", SDL_SCANCODE_LSHIFT, -1);
		//is->AddAxisMap("LookUp", Special::MOUSEMOVE_YVEL, -1);   //turn up
		//is->AddAxisMap("LookRight", Special::MOUSEMOVE_XVEL, -1);
		//is->AddAxisMap("SpawnTest", SDL_SCANCODE_G);		//press g to spawn objects
		////bind controls
		//is->BindAxis("MoveForward", ExternalMove);
		//is->BindAxis("MoveRight", ExternalMoveRight);
		//is->BindAxis("MoveUp", ExternalMoveUp);
		//is->BindAxis("LookUp", ExternalLookUp);
		//is->BindAxis("LookRight", ExternalLookRight);

		//is->BindAxis("SpawnTest", SpawnEntities);
		//is->BindAction("Click", click, ActionState::Released);
		GameplayStatics::inputManager = is;

		//create a world
		GameplayStatics::currentWorld = new TestWorld();
	}

	int OnShutdown() override {


		return 0;
	}
};
