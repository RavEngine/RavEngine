
#include "RavEngine_App.h"
#include <GameplayStatics.hpp>

int RavEngine_App::run(int argc, char** argv) {

	//invoke startup hook
	OnStartup(argc, argv);

	//in loop tick related things
	{
		//window events to feed to input manager

		//process loaded inputs
		GameplayStatics::inputManager->tick();

		//tick the world
		GameplayStatics::currentWorld->tick();

		//render frame
	}

	//invoke shutdown
	return OnShutdown();
}