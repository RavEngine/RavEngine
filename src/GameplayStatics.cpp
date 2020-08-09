//
//  GameplayStatics.cpp
//  RavEngine_Static
//
//  Copyright © 2020 Ravbug. All rights reserved.
//

#include "GameplayStatics.hpp"

#include <SDL.h>
#include <SDL_syswm.h>

//startup world
Ref<World> GameplayStatics::currentWorld;
GameplayStatics::vs GameplayStatics::VideoSettings;
Ref<InputSystem> GameplayStatics::inputManager;
int GameplayStatics::width = VideoSettings.width, GameplayStatics::height = VideoSettings.height;
GameplayStatics::static_constructor GameplayStatics::statcons;
//SDL_Window* GameplayStatics::mainWindow = nullptr;
OgreStatics GameplayStatics::ogreFactory;

using namespace std;

GameplayStatics::static_constructor::static_constructor()
{
	
}
