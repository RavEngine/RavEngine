//
//  GameplayStatics.hpp
//  RavEngine_Static
//
//  Copyright © 2020 Ravbug. All rights reserved.
//

#pragma once
#include "World.hpp"
#include <string>
#include "InputManager.hpp"

namespace RavEngine {
	struct GameplayStatics {
		static Ref<World> currentWorld;

		static Ref<InputManager> inputManager;

		struct vs {
			int width = 960; int height = 540;
			bool vsync = true;
		};
		static vs VideoSettings;

	};
}
