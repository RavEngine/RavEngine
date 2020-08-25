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
#include <memory>
#include "WeakRef.hpp"

struct SDL_Window;
namespace RavEngine {
    class SDLSurface;

    class RenderEngine : public SharedObject {
    public:
        virtual ~RenderEngine();
        void Spawn(Ref<Entity> e);
        void Destroy(Ref<Entity> e);
        RenderEngine(const WeakRef<World>& w);
        void Draw();

        static const std::string currentBackend();
        WeakRef<World> world;

        static SDL_Window* const GetWindow();

        void resize();

    protected:

        static void Init();

        static std::shared_ptr<RavEngine::SDLSurface> surface;
    };
}
