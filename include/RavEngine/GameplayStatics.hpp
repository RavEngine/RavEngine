//
//  GameplayStatics.hpp
//  RavEngine_Static
//
//  Copyright © 2020 Ravbug. All rights reserved.
//

#pragma once
#include "World.hpp"
#include <string>
#include "InputSystem.h"

struct GameplayStatics{
	static Ref<World> currentWorld;

	static Ref<InputSystem> inputManager;
	
	struct vs{
		int width = 960; int height = 540;
		bool vsync = true;
	};
	static vs VideoSettings;

	//current width and height
	static int width, height;

	//static constructor
	//this ensures that bgfx and sdl are created and initialized before anything else runs
	static struct static_constructor {
		static_constructor();
	};
	static static_constructor statcons;
	//static SDL_Window* mainWindow;
	
	//static void StartGame(Ref<World>, const std::string&);
};
