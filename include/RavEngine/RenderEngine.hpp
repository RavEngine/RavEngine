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

namespace LLGL {
    class RenderSystem;
    class CommandQueue;
    class CommandBuffer;
}

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

        //get reference to the render system - for internal use only
        static std::unique_ptr<LLGL::RenderSystem>& GetRenderSystem() {
            return renderer;
        }

        static std::shared_ptr<RavEngine::SDLSurface> GetSurface() {
            return surface;
        }

    protected:

        static void Init();

        static std::shared_ptr<RavEngine::SDLSurface> surface;
        static std::unique_ptr<LLGL::RenderSystem> renderer;

        LLGL::CommandQueue* queue = nullptr;
        LLGL::CommandBuffer* commands = nullptr;;
    };
}
