//
//  GameplayStatics.cpp
//  RavEngine_Static
//
//  Copyright © 2020 Ravbug. All rights reserved.
//

#include "GameplayStatics.hpp"
using namespace RavEngine;


//#include <SDL.h>
//#include <SDL_syswm.h>

//startup world
Ref<RavEngine::World> GameplayStatics::currentWorld;
GameplayStatics::vs GameplayStatics::VideoSettings;
Ref<InputSystem> GameplayStatics::inputManager;
int GameplayStatics::width = VideoSettings.width, GameplayStatics::height = VideoSettings.height;
GameplayStatics::static_constructor GameplayStatics::statcons;
//SDL_Window* GameplayStatics::mainWindow = nullptr;

using namespace std;

GameplayStatics::static_constructor::static_constructor()
{
	
}
