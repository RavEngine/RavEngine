//
//  RenderEngine.hpp
//  RavEngine_Static
//
//  Copyright © 2020 Ravbug. All rights reserved.
//

#pragma once
#include "SharedObject.hpp"
#include "Entity.hpp"
#include <list>
#include "WeakRef.hpp"

struct SDL_Window;
namespace RavEngine {
    class RenderEngine : public SharedObject {
    public:
        virtual ~RenderEngine();
        void Spawn(Ref<Entity> e);
        void Destroy(Ref<Entity> e);
        RenderEngine(const WeakRef<World>& w);
        void Draw();

        static const std::string currentBackend();
        WeakRef<World> world;

        static SDL_Window* const GetWindow() {
            return window;
        }

        void resize();

    protected:

        static void Init();
		
		struct WindowSize{
			unsigned int width = 0, height = 0;
		};
		
		static WindowSize GetDrawableArea();

        static SDL_Window* window;
    };
}
