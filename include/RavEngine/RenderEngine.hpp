//
//  RenderEngine.hpp
//  RavEngine_Static
//
//  Copyright © 2020 Ravbug. All rights reserved.
//

#pragma once
#include "SharedObject.hpp"
#include "Entity.hpp"
#include "WeakRef.hpp"
#include <bgfx/bgfx.h>

struct SDL_Window;

namespace RavEngine {
    class SDLSurface;
	class DeferredGeometryMaterialInstance;

    class RenderEngine : public SharedObject {
    public:
        virtual ~RenderEngine();
        RenderEngine();
        void Draw(Ref<World>);

        static const std::string currentBackend();

		static SDL_Window* const GetWindow(){
			return window;
		}
		
		/**
		 Determines if the current hardware can support deferred rendering
		 @return true if the current hardware supports deferred
		 */
		static bool DeferredSupported();

        void resize();
		
		static struct vs {
			int width = 960; int height = 540;
			bool vsync = true;
		} VideoSettings;
		
    protected:
		static SDL_Window* window;
        static void Init();
		static uint32_t GetResetFlags();
    };
}
